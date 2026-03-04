/* See LICENSE file for copyright and license details. */
#include <sys/mount.h>
#include <sched.h>
#include <libsimple.h>
#include <libsimple-arg.h>

NUSAGE(125, "utility [argument] ...");


int
main(int argc, char *argv[])
{
	int fds[2], status;
	pid_t pid;
	char *dir = NULL;
	size_t dirsize = 0;
	size_t dirlen = 0;
	ssize_t r;

	libsimple_default_failure_exit = 125;

	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if (!argc)
		usage();

	if (unshare(CLONE_NEWNS))
		eprintf("unshare CLONE_NEWNS:");
	if (mount("none", "/", NULL, MS_REC | MS_SLAVE, NULL))
		eprintf("mount none / NULL MS_REC|MS_SLAVE NULL:");

	if (pipe(fds))
		eprintf("pipe:");
	pid = fork();
	if (pid < 0)
		eprintf("fork:");
	if (pid == 0) {
		if (setegid(getgid()))
			eprintf("setegid <real group>:");
		if (seteuid(getuid()))
			eprintf("seteuid <real user>:");

		close(fds[0]);
		if (fds[1] != STDOUT_FILENO) {
			if (dup2(fds[1], STDOUT_FILENO) != STDOUT_FILENO)
				eprintf("dup2 <pipe> STDOUT_FILENO:");
			close(fds[1]);
		}
		execlp(GIT_PATH, "git", "rev-parse", "--show-toplevel", NULL);
		eprintf("execlp %s:", GIT_PATH);
	}
	close(fds[1]);
	for (;;) {
		if (dirlen == dirsize)
			dir = erealloc(dir, dirsize += 512u);
		r = read(fds[0], &dir[dirlen], dirsize - dirlen);
		if (r <= 0) {
			if (!r)
				break;
			if (errno == EINTR)
				continue;
			eprintf("read <pipe>:");
		}
		dirlen += (size_t)r;
		if (!dirlen || dir[--dirlen] != '\n')
			eprintf("received invalid output from `git rev-parse --show-toplevel`");
	}
	close(fds[0]);
	if (waitpid(pid, &status, 0) != pid)
		eprintf("waitpid <subprocess>:");
	if (status) {
		if (WIFSIGNALED(status))
			eprintf("subprocess git was killed by signal %i\n", WTERMSIG(status));
		exit(libsimple_default_failure_exit);
	}
	if (dirsize - dirlen < sizeof("/.git"))
		dir = erealloc(dir, dirlen + sizeof("/.git"));
	stpcpy(&dir[dirlen], "/.git");

	if (mount(dir, dir, NULL, MS_BIND, NULL))
		eprintf("mount %s %s NULL MS_BIND NULL:", dir, dir);
	if (mount("none", dir, NULL, MS_BIND | MS_REMOUNT | MS_RDONLY, NULL))
		eprintf("mount none %s NULL MS_BIND|MS_REMOUNT|MS_RDONLY NULL:", dir);

	free(dir);

	if (setegid(getgid()))
		eprintf("setegid <real group>:");
	if (seteuid(getuid()))
		eprintf("seteuid <real user>:");

	execvp(argv[0], argv);
	enprintf(errno == ENOENT ? 127 : 126, "execvp %s:", argv[0]);
	return 0;
}

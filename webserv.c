/* server.c - a minimal web server
* usage: tinyweb portnumber
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define BACKLOG 1
int server_socket(int, int);

main(int argc, char *argv[])
{
	int sock, fd;
	FILE *fpin;
	char request[BUFSIZ];
	
	if (1 == argc ) {
		fprintf(stderr, "usage: ws portnum\n");	
		exit(1);
	}

	sock = server_socket(atoi(argv[1]), BACKLOG);
	if (-1 == sock) {
		exit(2);
	}

	/* loop to listen */
	while(1) {
		/* take a call and buffer it */
		fd = accept(sock, NULL, NULL);
		fpin = fdopen(fd, "r");
		/* read request */
		fgets(request, BUFSIZ, fpin);
		printf("got a call: request = %s", request);
		read_til_crnl(fpin);

		/* do what client asks */
		process_request(request, fd);

		fclose(fpin);

	}
}		

/*---------------------------------------*
read_til_crnl(FILE *)
skip over all request info until a CRNL is seen
----------------------------------------*/
read_til_crnl(FILE * fp) {
	char buf[BUFSIZ];
	while(fgets(buf, BUFSIZ, fp) != NULL && strcmp(buf, "\r\n") != 0); 
}

/*---------------------------------------------------
deal with request and write replay to fd
handles request in a new process
---------------------------------------------------*/
process_request(char *rq, int fd) {
	char cmd[BUFSIZ],arg[BUFSIZ];
	/* create a new process and return if not child */
	if (fork() != 0) 
		return;
	
	strcpy(arg, "./");
	if (sscanf(rq, "%s %s", cmd, arg + 2) != 2)
		return;

	if (strcmp(cmd, "GET") != 0) 
		cannot_do(fd);
	else if (not_exist(arg))
		do_404(arg, fd);
	else if (isadir(arg))
		do_ls(arg, fd);
	else if (ends_in_cgi(arg))
		do_exec(arg, fd);
	else
		do_cat(arg, fd);
}

/*--------------------------------------------
replay header
--------------------------------------------*/
header(FILE *fp, char *content_type) {
	fprintf(fp, "HTTP/1.0 200 OK\r\n");
	if (content_type) 
		fprintf(fp, "Content-type: %s\r\n", content_type);
}

/*------------------------------------------
not support
------------------------------------------*/
cannot_do(int fd) {
	FILE *fp = fdopen(fd, "w");

	fprintf(fp, "HTTP/1.0 501 Not Implement\r\n");
	fprintf(fp, "Content-type:text/plain\r\n");
	fprintf(fp, "\r\n");
	fprintf(fp, "That command not supported yet\r\n");
	fclose(fp);
}

/*-------------------------------------------
404 page not found
-------------------------------------------*/
do_404(char *item, int fd) {
	FILE *fp = fdopen(fd, "w");

	fprintf(fp, "HTTP/1.0 404 Not Found\r\n");
	fprintf(fp, "Content-type:text/plain\r\n");
	fprintf(fp, "\r\n");
	fprintf(fp, "That page you request is not found!\r\n");
	fclose(fp);
}

/*-------------------------------------------
The directory listing section
-------------------------------------------*/
isadir(char *f) {
	struct stat info;
	return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode));
}

not_exist(char *f) {
	struct stat info;
	return (stat(f, &info) == -1);
}

do_ls(char *dir, int fd) {
	FILE *fp;
	
	fp = fdopen(fd, "w");
	header(fp, "text/plain");
	fprintf(fp, "\r\n");
	fflush(fp);
	
	dup2(fd, 1);
	dup2(fd, 2);
	fclose(fp);
	close(fd);

	
	execlp("ls", "ls", "-1", dir, NULL);

	perror(dir);
	exit(1);
}	
	
/*----------------------------------------------
run cgi stuff
----------------------------------------------*/
char * file_type(char * f) {
	char *cp;
	if ((cp = strchr(f, '.')) != NULL)
		return cp + 1;
	return "";
}

ends_in_cgi(char *f) {
	return (strcmp(file_type(f), "cgi") == 0);
}

do_exec(char *prog, int fd) {
	FILE *fp;
	
	fp = fdopen(fd, "w");
	header(fp, NULL);
	fflush(fp);
	dup2(fd, 1);
	dup2(fd, 2);
	fclose(fp);
	close(fd);

	execl(prog, prog, NULL);
	perror(prog);
}

/*-------------------------------------------
do_cat(filename, fd)
sends back contents after a header	
-------------------------------------------*/
do_cat(char * f , int fd) {
	char * extension = file_type(f);
	char * content = "text/plain";
	FILE * fpsock, *fpfile;
	int c;

	if (strcmp(extension, "html") == 0)
		content = "text/html";
	else if (strcmp(extension, "gif") == 0)
		content = "text/gif";
	else if (strcmp(extension, "jpg") == 0)
		content = "text/jpg";
	else if (strcmp(extension, "jpeg") == 0)
		content = "text/jpeg";
	
	fpsock = fdopen(fd, "w");
	fpfile = fopen(f, "r");
	if (fpsock != NULL && fpfile != NULL)
	{
		header(fpsock, content);
		fprintf(fpsock, "\r\n");
		while ((c = getc(fpfile)) != EOF)
			putc(c, fpsock);
		fclose(fpsock);
		fclose(fpfile);
	}
	exit(0);

}



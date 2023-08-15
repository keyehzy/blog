#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>

char *filename;

void make_tmpfile(char *s, int n)
{
  srand(time(0));  
  int i;
  for (i = 0; i < n; i++) {
    s[i] = (char) (rand() % 78) + 48;
  }
  s[i] = '\0';
}

int make_new_entry_using_external_editor(void)
{
  int m = 5;
  char tmpfile[16] = "/tmp/";
  make_tmpfile(tmpfile + m, sizeof tmpfile - m - 1);

  int pid = fork();

  if (pid  == -1) {
    perror("can't fork");
    return 1;
  } else if (pid == 0) {
    char *args[] = { "vi", tmpfile, NULL };
    execv("/usr/bin/vi", args);
  } else {
    wait(NULL);

    FILE *f_dst = 0;;
    FILE *f_tmp = 0;

    if ((f_tmp = fopen(tmpfile, "r"))) {
      if ((f_dst = fopen(filename, "a"))) {
	
	time_t tm = time(0);
	char *p = ctime(&tm);

	while (*p) {
	  fputc(*p != '\n' ? *p : '\t', f_dst);
	  p++;
	}

	int i;
	while ((i = fgetc(f_tmp)) != EOF) {
	  if (i != ' ' && i != '\n') break;
	}

	ungetc(i, f_tmp);
	
	while ((i = fgetc(f_tmp)) != EOF) {
	  putc(i, f_dst);
	}
      } else {
	perror("can't open filename");
	return 1;
      }
    } else {
      perror("can't open tmpfile");
      return 1;
    }

    if (f_dst) fclose(f_dst);
    if (f_tmp) fclose(f_tmp);    
  }
  return 0;
}

int show_notes(void)
{
  FILE *fp = 0;
  if ((fp = fopen(filename, "r"))) {
    int i;    
    while ((i = fgetc(fp)) != EOF) {
      putchar(i);
    }
  } else {
    perror("can't open filename");
    return 1;
  }
  if (fp) fclose(fp);
  return 0;
}

int write_notes(int beg, int argc, char **argv)
{
  FILE *fp = 0;
  if ((fp = fopen(filename, "a"))) {
    time_t tm = time(0);
    char *p = ctime(&tm);

    while (*p) {
      fputc(*p != '\n' ? *p : '\t', fp);
      p++;
    }

    for (int i = beg + 1; i < argc; i++) {
      fprintf(fp, "%s%c", argv[i], 1 + i - argc ? ' ' : '\n');
    }
  } else {
    perror("can't open filename");
    return 1;
  }
  if (fp) fclose(fp);
  return 0;
}

int help(void) {
  printf("usage: notes [options] [command]\n\n");

  printf("commands:\n");
  printf(" show - Display the notes file contents\n");
  printf(" write [text] - Add text to the notes file\n");
  printf(" help - Display this help message\n\n");

  printf("options:\n");
  printf(" -f filename - Specify notes file location (default is $HOME/notes.txt)\n");

  printf("\nIf no command is specified, notes will open an editor to create a new note.\n");
  return 0;
}

int main(int argc, char **argv)
{
  char *notes_file = "/notes.txt";
  char *homedir = getenv("HOME");
  filename = malloc(strlen(homedir) + strlen(notes_file) + 1);
  strcpy(filename, homedir);
  strcat(filename, notes_file);
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      // options
      if (!(strcmp(argv[i], "-f")))
	filename = argv[++i];

      // actions
      if (!(strcmp(argv[i], "show")))
	return show_notes();
      else if (!(strcmp(argv[i], "write")))
	return write_notes(i, argc, argv);
      else if (!(strcmp(argv[i], "help")))
	return help();
    }
  }
  return make_new_entry_using_external_editor();    
}

/*
   Copyright (C) 1999-2002 Keiichi SAKAI.

   This is wrapper for gogo with cdda2wav.
   Gogo and cdda2wav process are connected with pair of socket for performance 
reason.

   cdda2mp3 [-silent] [-q quality] [-a] [-D device_spec] [-I interface_spec] [-v VB
R_rate] [-b bitrate] [prefix [track_number,...]]
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/* for socket */
#include <sys/types.h>
#include <sys/socket.h>

char opt_sgdev[32] = "/dev/sg0";
char opt_cddev[32] = "/dev/scd0";
char opt_if[32] = "generic_scsi" /* "cooked_ioctl" */;
char opt_track[32] = "00";

const char *opt_quality = NULL /* "0-9" */;
const char *opt_silent = NULL /* "-silent" */;
const char *opt_vbr = NULL;  /* 0-9 */
const char *opt_b = NULL;
const char *opt_a = NULL;

char const *gogo[32] = {
   "gogo",
};

char * const cdda2wav[] = {
   "cdda2wav",
   "-D",
   opt_sgdev,
   "-I",
   opt_if,
   "-A",
   opt_cddev,
   "-H",
   "-q",
   "-Owav",
   "-P0",
   "-t",
   opt_track,
   "-",
   NULL
};

char * const list_audio[] = {
   "list_audio_tracks",
   "-D",
   opt_sgdev,
   "-I",
   opt_if,
   "-A",
   opt_cddev,
   NULL
};

static int dev_null;

static int
cdda2mp3(const char *prefix, char *track)
{
   int pipe_fd[2];
   int status;
   char    *mp3file;

#if 1
   if(socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, pipe_fd)){
       fprintf(stderr, "socketpair() failed.\n");
       return 1;
   }
#else
   if(pipe(pipe_fd)){
       fprintf(stderr, "pipe() failed.\n");
       return 1;
   }
#endif
   if(fork() == 0){
       /* ripper process */
       close(pipe_fd[0]);
       dup2(dev_null, 2);
       dup2(pipe_fd[1], 1);
       close(dev_null);
       close(pipe_fd[1]);
       strncpy(opt_track, track, (sizeof opt_track) - 1);
       execvp(cdda2wav[0], cdda2wav);
       fprintf(stderr, "execvp(\"cdda2wav\") failed.\n");
       return 1;
   }
   if(fork() == 0){
       int argc = 1;

       /* encoder process */
       close(pipe_fd[1]);
       dup2(pipe_fd[0], 0);
       close(pipe_fd[0]);
       mp3file = alloca(strlen(prefix)+strlen(track)+5);
       strcpy(mp3file, prefix);
       strcat(mp3file, track);
       strcat(mp3file, ".mp3");
       if(opt_silent != NULL){
           gogo[argc++] = opt_silent;
       }
       if(opt_quality != NULL){
           gogo[argc++] = "-q";
           gogo[argc++] = opt_quality;
       }
       if(opt_a != NULL){
           gogo[argc++] = "-a";
       }
       if(opt_b != NULL){
           gogo[argc++] = "-b";
           gogo[argc++] = opt_b;
       }
       if(opt_vbr != NULL){
           gogo[argc++] = "-v";
           gogo[argc++] = opt_vbr;
       }
       gogo[argc++] = "stdin";
       gogo[argc++] = mp3file;
       gogo[argc++] = NULL;
       execvp(gogo[0], gogo);
       fprintf(stderr, "execvp(\"gogo\") failed.\n");
       return 1;
   }
   close(pipe_fd[0]);
   close(pipe_fd[1]);
   wait(&status);
   wait(&status);

   return status;
}

int
main(int argc, char *argv[])
{
   int pipe_fd[2];
   int status;
   char    buffer[4096];
   int length;
   char    *track, *sector;
   const char *prefix = "audiotrack";
   int i;

   if((dev_null = open("/dev/null", O_WRONLY)) < 0){
       fprintf(stderr, "open(\"/dev/null\") failed.\n");
       return 1;
   }

   i = 1;
   while(i < argc){
       if(argv[i][0] == '-'){
           if(strcmp(argv[i], "-silent") == 0){
               opt_silent = "-silent";
           }else if(strcmp(argv[i], "-A") == 0){
               i++;
               strncpy(opt_cddev, argv[i], (sizeof opt_cddev) - 1);
           }else if(strcmp(argv[i], "-D") == 0){
               i++;
               strncpy(opt_sgdev, argv[i], (sizeof opt_sgdev) - 1);
           }else if(strcmp(argv[i], "-I") == 0){
               i++;
               strncpy(opt_if, argv[i], (sizeof opt_if) - 1);
           }else if(strcmp(argv[i], "-a") == 0){
               opt_a = "-a";
           }else if(strcmp(argv[i], "-q") == 0){
               opt_quality = argv[++i];
           }else if(strcmp(argv[i], "-b") == 0){
               opt_b = argv[++i];
           }else if(strcmp(argv[i], "-v") == 0){
               opt_vbr = argv[++i];
           }else if(strncmp(argv[i], "-A", 2) == 0){
               strncpy(opt_cddev, argv[i] + 2, (sizeof opt_cddev) - 1);
           }else if(strncmp(argv[i], "-D", 2) == 0){
               strncpy(opt_sgdev, argv[i] + 2, (sizeof opt_sgdev) - 1);
           }else if(strncmp(argv[i], "-I", 2) == 0){
               strncpy(opt_if, argv[i] + 2, (sizeof opt_if) - 1);
           }else if(strncmp(argv[i], "-b", 2) == 0){
               opt_b = argv[i] + 2;
           }else if(strncmp(argv[i], "-v", 2) == 0){
               opt_vbr = argv[i] + 2;
           }
       }else{
           prefix = argv[i];
           i++;
           break;
       }
       i++;
   }

   if(i < argc){
       /* encode specified tracks */
       while(i < argc){
           if(cdda2mp3(prefix, argv[i])){
               return 1;
           }
           i++;
       }
       return 0;
   }

   /* encode all tracks */
   if(pipe(pipe_fd)){
       fprintf(stderr, "pipe() failed.\n");
       return 1;
   }

   if(fork() == 0){
       /* child */
       close(pipe_fd[0]);
       dup2(dev_null, 2);
       dup2(pipe_fd[1], 1);
       close(dev_null);
       close(pipe_fd[1]);
       execvp(cdda2wav[0], list_audio);
       fprintf(stderr, "execvp(\"list_audio_tracks\") failed.\n");
       return 1;
   }
   close(pipe_fd[1]);
   wait(&status);
   if(status){
       fprintf(stderr, "can't get audio track list.\n");
       return status;
   }

   length = read(pipe_fd[0], buffer, sizeof(buffer) - 1);
   close(pipe_fd[0]);
   if(length <= 0){
       fprintf(stderr, "can't get audio track list.\n");
       return 1;
   }
   buffer[length] = 0;

   track = strtok(buffer, " \t\r\n");
   while((sector = strtok(NULL, " \t\r\n")) != NULL){
       if(cdda2mp3(prefix, track)){
           return 1;
       }
       track = strtok(NULL, " \t\r\n");
       if(track == NULL){
           break;
       }
   }
   return 0;
}


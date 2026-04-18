#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"

Node* head = NULL;

//check for terminated background processes
void check_zombies() {
    int status;
    pid_t pid;
    // WNOHANG makes sure nothing block the PMan prompt
    // waitpid(-1 = wait for any child process, &status = stores the exit report into ,WNOHANG = return immediately if no child has exited.)
    // &status can store the exit code(Ex: exit(5) stores 5), or store the fact that it was killed by a signal
    // waitpid returns a pid if a process has been killed/terminated, returns 0 if no process states change, -1 if error
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        //WIFEXITED is for if they exited normalled
        if (WIFEXITED(status)) {
            printf("Background process %d terminated.\n", pid);
        } else if (WIFSIGNALED(status)) {
        //WIFSIGNALED for if they are killed by signal
            printf("Background process %d was killed.\n", pid);
        }
        head = deleteNode(head, pid);
    }
}


void func_BG(char **cmd){
    pid_t pid = fork();

    if (pid == 0){
      if(execvp(cmd[1], &cmd[1]) < 0){
        //perror prints a message before the errror
        perror("Error on execvp");
      }
      exit(EXIT_SUCCESS);
    }
    else if (pid > 0) {
        // Parent process: Store the child info 
        printf("Started background process: %d\n", pid);
        // We need to store a copy of the path/name
        //s
        head = add_newNode(head, pid, strdup(cmd[1]));
    }
    else {
      perror("fork failed");
      exit(EXIT_FAILURE);
	  }
}


void func_BGlist(char **cmd){
    // Node * cur = head;
    // int count = 0;

    // while (cur != NULL) {
    //     printf("%d: %s\n", cur->pid, cur->cmd);
    //     cur = cur->next;
    //     count++;
    // }

    // printf("Total background jobs: %d\n", count);

    printList(head);
}


void func_BGkill(char * str_pid){
	//Your code here
  // pid_t pid = (pid_t)(str_pid - '0');
  if (str_pid == NULL) {
      return;
  };

  //atoi converts a numeric string into an integer value
  pid_t pid = atoi(str_pid);

    if (kill(pid, SIGTERM) == -1) {
        perror("bgkill failed");
    }
}


void func_BGstop(char * str_pid){
	//Your code here
  if (str_pid == NULL) {
      return;
  };
  //atoi converts a numeric string into an integer value
  pid_t pid = atoi(str_pid);

    if (kill(pid, SIGSTOP) == -1) {
        perror("bgstop failed");
    }
}


void func_BGstart(char * str_pid){
	//Your code here
  if (str_pid == NULL) {
      return; 
  };
  //atoi converts a numeric string into an integer value
  pid_t pid = atoi(str_pid);

    if (kill(pid, SIGCONT) == -1) {
        perror("bgstart failed");
    }
}


void func_pstat(char * str_pid){
    //Your code here

    if (str_pid == NULL) {
        return;
    };
    //atoi converts a numeric string into an integer value
    pid_t pid = atoi(str_pid);
    
    char path_stat[64];
    char path_status[64];
    //accessses /proc/pid/stat and /proc/pid/status, then stores it in path_stat and path_status
    sprintf(path_stat, "/proc/%d/stat", pid);
    sprintf(path_status, "/proc/%d/status", pid);

    //opens the file to read
    FILE *f_stat = fopen(path_stat, "r");
    if (f_stat == NULL) {
        printf("Error: Process %d does not exist.\n", pid);
        return;
    }

    //going through /proc/pid/stat
    char comm[256], state;
    long utime, stime, rss;
    //%s = string, %u = unsigned int, %d = int, %ld = long int, %c = char
    //%*s or %*d or %*u are assignment-suppression characters and skips the fields so that they don't get stored
    //& is needed as fscanf needs to change the internal values of the variables, otherwise they will bget sent a copy of the vars and change those
    fscanf(f_stat, "%*d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld %*d %*d %*d %*d %*d %*d %*u %*u %ld", 
           comm, &state, &utime, &stime, &rss);
    fclose(f_stat);

    //going through  /proc/pid/status for context switches
    FILE *f_status = fopen(path_status, "r");
    if (f_status == NULL){
      return;
    };
    char line[256];
    long vol = 0, nonvol = 0;
    //fgets(stores in line, 256 is the maximum characters to read, f_status is the FILE beign accessed)
    while (fgets(line, sizeof(line), f_status)) {
       //strncmp compares two strings according to the max amount of characters specified(24 and 27 here)
       //strncmp returns 0 if the strings are equal, negative if str1 < str2, positive if str1 > str2
        if (strncmp(line, "voluntary_ctxt_switches:", 24) == 0){
            //atol converts a string into a long int
            //atol also skips the specified 25 chars in the string, then convert the rest after that
            vol = atol(line + 25);
        } 
        if (strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0){
            nonvol = atol(line + 28);
        } 
    }
    fclose(f_status);

    //In Linux system, sysconf(_SC_CLK_TCK) returns 100 by default
    float ticks = sysconf(_SC_CLK_TCK);
    printf("comm: %s\n", comm);
    printf("state: %c\n", state);
    printf("utime: %f\n", utime / ticks);
    printf("stime: %f\n", stime / ticks);
    printf("rss: %ld\n", rss);
    printf("voluntary_ctxt_switches: %ld\n", vol);
    printf("nonvoluntary_ctxt_switches: %ld\n", nonvol);

}

 
int main(){
    char user_input_str[50];
    while (true) {
      check_zombies();

      printf("Pman: > ");
      fgets(user_input_str, 50, stdin);
      printf("User input: %s \n", user_input_str);
      char * ptr = strtok(user_input_str, " \n");
      if(ptr == NULL){
        continue;
      }
      char * lst[50];
      int index = 0;
      lst[index] = ptr;
      index++;
      while(ptr != NULL){
        ptr = strtok(NULL, " \n");
        lst[index]=ptr;
        index++;
      }
      if (strcmp("bg",lst[0]) == 0){
        func_BG(lst);
      } else if (strcmp("bglist",lst[0]) == 0) {
        func_BGlist(lst);
      } else if (strcmp("bgkill",lst[0]) == 0) {
        func_BGkill(lst[1]);
      } else if (strcmp("bgstop",lst[0]) == 0) {
        func_BGstop(lst[1]);
      } else if (strcmp("bgstart",lst[0]) == 0) {
        func_BGstart(lst[1]);
      } else if (strcmp("pstat",lst[0]) == 0) {
        func_pstat(lst[1]);
      } else if (strcmp("q",lst[0]) == 0) {
        printf("Bye Bye \n");
        exit(0);
      } else {
        printf("Invalid input\n");
      }
    }

  return 0;
}


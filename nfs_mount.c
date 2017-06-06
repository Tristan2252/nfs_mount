#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>

#define MAX_WAIT 5   // number of times ping is allowed to fail before and after tring to wake host
#define BOOT_WAIT 30 // max time in sec to wait for host to come up

void wake_host();

int main(int argc, char *argv[])
{

        int status;
        int cnt = 0; // count number of attempts

        /**
         * fork child proc that pings host 192.168.1.2 (nfs server) once to check to see 
         * if it is up. The error number of ping is stored in status by waitpid and used
         * to see if the ping was successful or not. If unsuccessful, the ping is repeated 
         * until MAX_WAIT is reached. When this occurs an etherwake is attempted on the 
         * host to try to bring it up. After the etherwake attempt the program sleeps 
         * for BOOT_WAIT sec's to allow the host to come up. After this another sequence 
         * of ping attempts occurs and is hopefully successful. If not program exits
         **/
PING:
        switch(fork()){
                case 0:
                        /* Child Code */
                        execl("/bin/ping", "ping", "-c 1", "192.168.1.2", NULL);
                        break;
                case -1:
                        perror("Error in fork: ");
                        break;
        }

        // wait for child to finish and load status to status var
        wait(&status);

        if (WEXITSTATUS(status)){
                if (cnt == MAX_WAIT){
                        dprintf(2, "\033[31;1mError mounting nfs share: host not found \033[0m\n");
                        wake_host();
                        sleep(BOOT_WAIT); // wait for host bootup
                        cnt++;
                        goto PING; // repeat ping process, this will allow even more time for host to come up
                } else if (cnt == (MAX_WAIT * 2)){
                        dprintf(2, "\033[31;1mError mounting nfs share: host still not found \033[0m\n");
                        _exit(-1);
                } else {
                        cnt++;
                        sleep(2);
                        printf("\033[33;1mRetry: %d\033[0m\n", cnt);
                        goto PING;
                }
        }

        if (mount(":/mnt/Movie-Drive", "/mnt/Movie-Drive", "nfs", 0, "addr=192.168.1.2")){
                perror("Error in mount");
        }
        return 0;
}

void wake_host(){
        int status;
        
        dprintf(2, "\033[31;1mAttempting to wake host...\033[0m");
        switch(fork()){
                case 0:
                        /* Child Proc */
                        execl("/usr/sbin/etherwake", "/usr/sbin/etherwake", "-i", "enp4s0", "00:1b:21:5d:d1:9e", NULL);
                        break;
                case -1:
                        perror("Error in fork: ");
        }

        wait(&status);

        if (WEXITSTATUS(status)){
                dprintf(2, "\033[31;1mUNSUCCESSFUL\033[0m\n");
                _exit(-1);
        } else {
                dprintf(2, "\033[32;1mSUCCESSFUL\033[0m\n");
        }
}


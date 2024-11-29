#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_PROC_NAME_LEN 256

// Function to get basic system information
void getSystemInfo() {
    struct utsname sys_info;
    uname(&sys_info);
    printf("OS Release: %s\n", sys_info.release);
    printf("Kernel Version: %s\n", sys_info.version);

    struct statvfs stat;
    statvfs("/", &stat);
    printf("Total Disk Space: %.2f GB\n", (double)stat.f_blocks * stat.f_frsize / (1024.0 * 1024.0 * 1024.0));
    printf("Available Disk Space: %.2f GB\n", (double)stat.f_bavail * stat.f_frsize / (1024.0 * 1024.0 * 1024.0));

    // Add code to get memory and processor information
}

// Function to list running processes
void listProcesses() {
    DIR *dir;
    struct dirent *ent;
    char proc_name[MAX_PROC_NAME_LEN];

    dir = opendir("/proc");
    if (dir == NULL) {
        perror("Error opening /proc directory");
        return;
    }

    printf("PID\tName\tUser\tState\tMemory\tCPU Time\tStarted\n");

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) {
            int pid = atoi(ent->d_name);
            sprintf(proc_name, "/proc/%d/stat", pid);
            int fd = open(proc_name, O_RDONLY);
            if (fd != -1) {
                // Read process information from /proc/<pid>/stat
                close(fd);
            }
            // Add code to get process-specific information and print it
        }
    }

    closedir(dir);
}

// Function to display process-specific actions
void processActions(int pid) {
    printf("Actions for process %d:\n", pid);
    printf("1. Stop\n");
    printf("2. Continue\n");
    printf("3. Kill\n");
    printf("4. List memory maps\n");
    printf("5. List open files\n");
    // Add code to implement the actions
}

// Function to display detailed process information
void displayProcessDetails(int pid) {
    (void)pid;
    // Add code to read and display detailed process information from /proc/<pid>/*
}

// Function to display usage graphs
void displayUsageGraphs() {
    // Add code to read and display CPU, memory, swap, and network usage graphs
}

// Function to display file system usage
void displayFileSystemUsage() {
    // Add code to read and display file system usage for each mount point
}

int main() {
    getSystemInfo();
    listProcesses();

    int choice, pid;
    while (1) {
        printf("\nTask Manager Menu:\n");
        printf("1. Process Information\n");
        printf("2. Process-specific Actions\n");
        printf("3. Detailed Process View\n");
        printf("4. Usage Graphs\n");
        printf("5. File System\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                listProcesses();
                break;
            case 2:
                printf("Enter process ID: ");
                scanf("%d", &pid);
                processActions(pid);
                break;
            case 3:
                printf("Enter process ID: ");
                scanf("%d", &pid);
                displayProcessDetails(pid);
                break;
            case 4:
                displayUsageGraphs();
                break;
            case 5:
                displayFileSystemUsage();
                break;
            case 6:
                printf("Exiting Task Manager...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
}

#ifndef FACULTY_FUNCTIONS
#define FACULTY_FUNCTIONS

// Semaphores are necessary joint account due the design choice I've made
#include <sys/ipc.h>
#include <sys/sem.h>

#include "./common.h"

struct Faculty loggedInFaculty;
int semIdentifier;

// Function Prototypes =================================

bool faculty_operation_handler(int connFD);
// bool deposit(int connFD);
// bool withdraw(int connFD);
// bool get_balance(int connFD);
bool change_password_faculty(int connFD);
// bool lock_critical_section(struct sembuf *semOp);
// bool unlock_critical_section(struct sembuf *sem_op);
// void write_transaction_to_array(int *transactionArray, int ID);
// int write_transaction_to_file(int accountNumber, long int oldBalance, long int newBalance, bool operation);

// =====================================================

// Function Definition =================================

bool faculty_operation_handler(int connFD)
{
    if (login_handler(false, true, connFD, NULL, &loggedInFaculty))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1024], writeBuffer[1024]; // A buffer used for reading & writing to the client

        // Get a semaphore for the student
        key_t semKey = ftok(FACULTY_FILE, loggedInFaculty.id); // Generate a key based on the account number hence, different customers will have different semaphores

        union semun
        {
            int val; // Value of the semaphore
        } semSet;

        int semctlStatus;
        semIdentifier = semget(semKey, 1, 0); // Get the semaphore if it exists
        if (semIdentifier == -1)
        {
            semIdentifier = semget(semKey, 1, IPC_CREAT | 0700); // Create a new semaphore
            if (semIdentifier == -1)
            {
                perror("Error while creating semaphore!");
                exit(1);
            }

            semSet.val = 1; // Set a binary semaphore
            semctlStatus = semctl(semIdentifier, 0, SETVAL, semSet);
            if (semctlStatus == -1)
            {
                perror("Error while initializing a binary sempahore!");
                exit(1);
            }
        }

        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_LOGIN_SUCCESS);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, FACULTY_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing FACULTY_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading student's choice for FACULTY_MENU");
                return false;
            }
            
            
            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                //enroll_courses(connFD, loggedInCustomer.id);
                break;
            case 2:
                //unenroll_courses(connFD, loggedInCustomer.id);
                break;
            case 3:
                //get_student_cource_details(connFD, loggedInCustomer.id);
                break;
            case 4:
                get_faculty_details(connFD, loggedInFaculty.id);
                break;
            case 5:
                change_password_faculty(connFD);
                break;
            default:
                writeBytes = write(connFD, FACULTY_LOGOUT, strlen(FACULTY_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        // STUDENT LOGIN FAILED
        return false;
    }
    return true;
}


//Change Password =====================================================================================================================
bool change_password_faculty(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    char newPassword[1024];

    // Lock the critical section
    struct sembuf semOp = {0, -1, SEM_UNDO};
    int semopStatus = semop(semIdentifier, &semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while locking critical section");
        return false;
    }

    writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS, strlen(PASSWORD_CHANGE_OLD_PASS));
    if (writeBytes == -1)
    {
        perror("Error writing PASSWORD_CHANGE_OLD_PASS message to Faculty!");
        unlock_critical_section(semIdentifier,&semOp);
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading old password response from Faculty");
        unlock_critical_section(semIdentifier,&semOp);
        return false;
    }

    //if (strcmp(crypt(readBuffer, SALT_BAE), loggedInCustomer.password) == 0)
    if (strcmp(readBuffer, loggedInFaculty.password) == 0)
    {
        // Password matches with old password
        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS, strlen(PASSWORD_CHANGE_NEW_PASS));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS message to Faculty!");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading new password response from Faculty");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }

        // strcpy(newPassword, crypt(readBuffer, SALT_BAE));
        strcpy(newPassword, readBuffer);

        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_RE, strlen(PASSWORD_CHANGE_NEW_PASS_RE));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS_RE message to Faculty!");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading new password reenter response from Faculty");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }

        // if (strcmp(crypt(readBuffer, SALT_BAE), newPassword) == 0)
        if (strcmp(readBuffer, newPassword) == 0)
        {
            // New & reentered passwords match

            strcpy(loggedInFaculty.password, newPassword);

            int facultyFileDescriptor = open(FACULTY_FILE, O_WRONLY);
            if (facultyFileDescriptor == -1)
            {
                perror("Error opening customer file!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            off_t offset = lseek(facultyFileDescriptor, loggedInFaculty.id * sizeof(struct Faculty), SEEK_SET);
            if (offset == -1)
            {
                perror("Error seeking to the customer record!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};
            int lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining write lock on student record!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            writeBytes = write(facultyFileDescriptor, &loggedInFaculty, sizeof(struct Faculty));
            if (writeBytes == -1)
            {
                perror("Error storing updated Faculty password into Faculty record!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            lock.l_type = F_UNLCK;
            lockingStatus = fcntl(facultyFileDescriptor, F_SETLK, &lock);

            close(facultyFileDescriptor);

            writeBytes = write(connFD, PASSWORD_CHANGE_SUCCESS, strlen(PASSWORD_CHANGE_SUCCESS));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

            unlock_critical_section(semIdentifier,&semOp);

            return true;
        }
        else
        {
            // New & reentered passwords don't match
            writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_INVALID, strlen(PASSWORD_CHANGE_NEW_PASS_INVALID));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        }
    }
    else
    {
        // Password doesn't match with old password
        writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS_INVALID, strlen(PASSWORD_CHANGE_OLD_PASS_INVALID));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    }

    unlock_critical_section(semIdentifier,&semOp);

    return false;
}

// =====================================================

#endif
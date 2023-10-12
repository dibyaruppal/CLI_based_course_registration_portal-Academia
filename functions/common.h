#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

#include <stdio.h>     // Import for `printf` & `perror`
#include <unistd.h>    // Import for `read`, `write & `lseek`
#include <string.h>    // Import for string functions
#include <stdbool.h>   // Import for `bool` data type
#include <sys/types.h> // Import for `open`, `lseek`
#include <sys/stat.h>  // Import for `open`
#include <fcntl.h>     // Import for `open`
#include <stdlib.h>    // Import for `atoi`
#include <errno.h>     // Import for `errno`

#include "../record-struct/course.h"
#include "../record-struct/faculty.h"
#include "../record-struct/student.h"
#include "./admin-credential.h"
#include "./server-constant.h"

// Function Prototypes =================================

bool login_handler(bool isAdmin, int connFD, struct Student *ptrToStudent);
// bool get_account_details(int connFD, struct Account *customerAccount);
bool get_student_details(int connFD, int studentID);
bool get_faculty_details(int connFD, int facultyID);
// bool get_transaction_details(int connFD, int accountNumber);

// =====================================================

// Function Definition =================================

bool login_handler(bool isAdmin, int connFD, struct Student *ptrToStudentID)
{
    ssize_t readBytes, writeBytes;            // Number of bytes written to / read from the socket
    char readBuffer[1024], writeBuffer[1024]; // Buffer for reading from / writing to the client
    char tempBuffer[1024];
    struct Student student;

    int ID;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type
    if (isAdmin)
        strcpy(writeBuffer, ADMIN_LOGIN_WELCOME);
    else
        strcpy(writeBuffer, STUDENT_LOGIN_WELCOME);

    // Append the request for LOGIN ID message
    strcat(writeBuffer, "\n");
    strcat(writeBuffer, LOGIN_ID);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing WELCOME & LOGIN_ID message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading login ID from client!");
        return false;
    }

    bool userFound = false;

    if (isAdmin)
    {
        if (strcmp(readBuffer, ADMIN_LOGIN_ID) == 0)
            userFound = true;
    }
    // else
    // {
    //     bzero(tempBuffer, sizeof(tempBuffer));
    //     strcpy(tempBuffer, readBuffer);
    //     strtok(tempBuffer, "-");
    //     ID = atoi(strtok(NULL, "-"));

    //     int customerFileFD = open(CUSTOMER_FILE, O_RDONLY);
    //     if (customerFileFD == -1)
    //     {
    //         perror("Error opening customer file in read mode!");
    //         return false;
    //     }

    //     off_t offset = lseek(customerFileFD, ID * sizeof(struct Student), SEEK_SET);
    //     if (offset >= 0)
    //     {
    //         struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Customer), sizeof(struct Customer), getpid()};

    //         int lockingStatus = fcntl(customerFileFD, F_SETLKW, &lock);
    //         if (lockingStatus == -1)
    //         {
    //             perror("Error obtaining read lock on customer record!");
    //             return false;
    //         }

    //         readBytes = read(customerFileFD, &customer, sizeof(struct Customer));
    //         if (readBytes == -1)
    //         {
    //             ;
    //             perror("Error reading customer record from file!");
    //         }

    //         lock.l_type = F_UNLCK;
    //         fcntl(customerFileFD, F_SETLK, &lock);

    //         if (strcmp(customer.login, readBuffer) == 0)
    //             userFound = true;

    //         close(customerFileFD);
    //     }
    //     else
    //     {
    //         writeBytes = write(connFD, CUSTOMER_LOGIN_ID_DOESNT_EXIT, strlen(CUSTOMER_LOGIN_ID_DOESNT_EXIT));
    //     }
    // }

    if (userFound)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, PASSWORD, strlen(PASSWORD));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == 1)
        {
            perror("Error reading password from the client!");
            return false;
        }

        // char hashedPassword[1000];
        // strcpy(hashedPassword, crypt(readBuffer, SALT_BAE));

        if (isAdmin)
        {
            // if (strcmp(hashedPassword, ADMIN_PASSWORD) == 0)
            //     return true;
            if (strcmp(readBuffer, ADMIN_PASSWORD) == 0)
                return true;
        }
        // else
        // {
        //     if (strcmp(readBuffer, customer.password) == 0)
        //     {
        //         *ptrToStudentID = customer;
        //         return true;
        //     }
        //     //original
        //     // if (strcmp(hashedPassword, customer.password) == 0)
        //     // {
        //     //     *ptrToCustomerID = customer;
        //     //     return true;
        //     // }
        // }

        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_PASSWORD, strlen(INVALID_PASSWORD));
    }
    else
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
    }

    return false;
}

bool get_student_details(int connFD, int studentID)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1024], writeBuffer[1024]; // A buffer for reading from / writing to the socket
    char tempBuffer[1024];

    struct Student student;
    int studentFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};

    if (studentID == -1)
    {
        writeBytes = write(connFD, GET_STUDENT_ID, strlen(GET_STUDENT_ID));
        if (writeBytes == -1)
        {
            perror("Error while writing GET_STUDENT_ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error getting student ID from client!");
            ;
            return false;
        }

        studentID = atoi(readBuffer);
    }

    studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1)
    {
        // Customer File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing STUDENT_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    int offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (errno == EINVAL)
    {
        // Customer record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, STUDENT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing STUDENT_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the student file!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student));
    if (readBytes == -1)
    {
        perror("Error reading student record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Student Details - \n\tID : %d\n\tName : %s\n\tGender : %c\n\tAge: %d\n\tDepartment: %s\n\tLoginID : %s\n\tActive : %d", student.id, student.name, student.gender, student.age, student.dept, student.login, student.active);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing student info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    return true;
}

bool get_faculty_details(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1024], writeBuffer[1024]; // A buffer for reading from / writing to the socket
    char tempBuffer[1024];

    struct Faculty faculty;
    int facultyFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Faculty), getpid()};

    if (facultyID == -1)
    {
        writeBytes = write(connFD, GET_FACULTY_ID, strlen(GET_FACULTY_ID));
        if (writeBytes == -1)
        {
            perror("Error while writing GET_FACULTY_ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error getting faculty ID from client!");
            ;
            return false;
        }

        facultyID = atoi(readBuffer);
    }

    facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        // Customer File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    int offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
    if (errno == EINVAL)
    {
        // Faculty record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, FACULTY_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing FACULTY_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required faculty record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the faculty file!");
        return false;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (readBytes == -1)
    {
        perror("Error reading faculty record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Faculty Details - \n\tID : %d\n\tName : %s\n\tDepartment: %s\n\tLoginID : %s", faculty.id, faculty.name, faculty.dept, faculty.login);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing faculty info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    return true;
}

// =====================================================

#endif
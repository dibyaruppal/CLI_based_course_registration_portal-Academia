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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>

#include "../record-struct/course.h"
#include "../record-struct/faculty.h"
#include "../record-struct/student.h"
#include "./admin-credential.h"
#include "./server-constant.h"

// Function Prototypes =================================

bool login_handler(bool isAdmin, bool isFaculty, int connFD, struct Student *ptrToStudent, struct Faculty *ptrToFaculty);
bool get_student_details(int connFD, int studentID);
bool get_faculty_details(int connFD, int facultyID);
bool get_all_course_details(int connFD);
bool lock_critical_section(int, struct sembuf *semOp);
bool unlock_critical_section(int, struct sembuf *sem_op);

// =====================================================

// Function Definition =================================

bool login_handler(bool isAdmin, bool isFaculty, int connFD, struct Student *ptrToStudentID, struct Faculty *ptrToFacultyID)
{
    ssize_t readBytes, writeBytes;            // Number of bytes written to / read from the socket
    char readBuffer[1024], writeBuffer[1024]; // Buffer for reading from / writing to the client
    char tempBuffer[1024];
    struct Student student;
    struct Faculty faculty;

    int ID=-1;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type
    if (isAdmin)
        strcpy(writeBuffer, ADMIN_LOGIN_WELCOME);
    else if (isFaculty)
        strcpy(writeBuffer, FACULTY_LOGIN_WELCOME);
    else
        strcpy(writeBuffer, STUDENT_LOGIN_WELCOME);

    // Append the request for LOGIN ID message
    strcat(writeBuffer, "\n");
    strcat(writeBuffer, LOGIN_ID);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing WELCOME & LOGIN_ID message to the user!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading login ID from user!");
        return false;
    }

    bool userFound = false;

    if (isAdmin)
    {
        if (strcmp(readBuffer, ADMIN_LOGIN_ID) == 0)
            userFound = true;
    }
    else if (isFaculty)
    {
        bzero(tempBuffer, sizeof(tempBuffer));
        strcpy(tempBuffer, readBuffer);
        strtok(tempBuffer, "-");
        ID = atoi(strtok(NULL, "-"));

        int facultyFileFD = open(FACULTY_FILE, O_RDONLY);
        if (facultyFileFD == -1)
        {
            perror("Error opening Faculty file in read mode!");
            return false;
        }

        off_t offset = lseek(facultyFileFD, ID * sizeof(struct Faculty), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Faculty), sizeof(struct Faculty), getpid()};

            int lockingStatus = fcntl(facultyFileFD, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on Faculty record!");
                return false;
            }

            readBytes = read(facultyFileFD, &faculty, sizeof(struct Faculty));
            if (readBytes == -1)
            {
                ;
                perror("Error reading Faculty record from file!");
            }

            lock.l_type = F_UNLCK;
            fcntl(facultyFileFD, F_SETLK, &lock);

            if (strcmp(faculty.login, readBuffer) == 0)
                userFound = true;

            close(facultyFileFD);
        }
        else
        {
            writeBytes = write(connFD, FACULTY_LOGIN_ID_DOESNT_EXIT, strlen(FACULTY_LOGIN_ID_DOESNT_EXIT));
        }
    }
    else
    {
        bzero(tempBuffer, sizeof(tempBuffer));
        strcpy(tempBuffer, readBuffer);
        strtok(tempBuffer, "-");
        ID = atoi(strtok(NULL, "-"));

        int studentFileFD = open(STUDENT_FILE, O_RDONLY);
        if (studentFileFD == -1)
        {
            perror("Error opening student file in read mode!");
            return false;
        }

        off_t offset = lseek(studentFileFD, ID * sizeof(struct Student), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Student), sizeof(struct Student), getpid()};

            int lockingStatus = fcntl(studentFileFD, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on student record!");
                return false;
            }

            readBytes = read(studentFileFD, &student, sizeof(struct Student));
            if (readBytes == -1)
            {
                ;
                perror("Error reading student record from file!");
            }

            lock.l_type = F_UNLCK;
            fcntl(studentFileFD, F_SETLK, &lock);

            if (strcmp(student.login, readBuffer) == 0)
                userFound = true;

            close(studentFileFD);
        }
        else
        {
            writeBytes = write(connFD, STUDENT_LOGIN_ID_DOESNT_EXIT, strlen(STUDENT_LOGIN_ID_DOESNT_EXIT));
        }
    }

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
        else if(isFaculty)
        {
            if (strcmp(readBuffer, faculty.password) == 0)
            {
                *ptrToFacultyID = faculty;
                return true;
            }
        }
        else
        {
            if(student.active == 0){
                bzero(writeBuffer, sizeof(writeBuffer));
                writeBytes = write(connFD, STUDENT_ACCOUNT_DEACTIVATED, strlen(STUDENT_ACCOUNT_DEACTIVATED));
                return false;
            }
            if (strcmp(readBuffer, student.password) == 0)
            {
                *ptrToStudentID = student;
                return true;
            }
            //original
            // if (strcmp(hashedPassword, customer.password) == 0)
            // {
            //     *ptrToCustomerID = customer;
            //     return true;
            // }
        }

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

// Fetching Student Details =====================================================================================================================
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
        // Student File doesn't exist
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
        // Student record doesn't exist
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

    close(studentFileDescriptor);
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

// Fetching Faculty Details =====================================================================================================================
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

// Fetching Course Details =====================================================================================================================
bool get_all_course_details(int connFD)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1024], writeBuffer[1024]; // A buffer for reading from / writing to the socket
    char tempBuffer[1024];

    struct Course course;
    int courseFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Course), getpid()};

    courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1)
    {
        // Course File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, COURSE_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing COURSE_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    // Fetching Number of courese records present
    int file_size = lseek(courseFileDescriptor, 0, SEEK_END);
    int number_of_courses = file_size/sizeof(struct Course);
    
    if(number_of_courses == 0)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, COURSE_NOT_AVAILABLE);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing COURSE_NOT_AVAILABLE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else
    {
        for(int cid=0;cid<number_of_courses;cid++)
        {
            
            int offset = lseek(courseFileDescriptor, cid * sizeof(struct Course), SEEK_SET);
            if (offset == -1)
            {
                perror("Error while seeking to required course record!");
                return false;   
            }
            lock.l_start = offset;

            int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error while obtaining read lock on the student file!");
                return false;
            }

            readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
            if (readBytes == -1)
            {
                perror("Error reading student record from file!");
                return false;
            }

            lock.l_type = F_UNLCK;
            fcntl(courseFileDescriptor, F_SETLK, &lock);
            bzero(writeBuffer, sizeof(writeBuffer));
            int available_seat = course.seat - course.alloted_seat;
            if(course.active == 1 && available_seat>0)
            {
                sprintf(writeBuffer, "\t-Course ID : %d\tCourse Name : %s\tAvailable Seats: %d^", course.course_id, course.course_name, course.seat - course.alloted_seat);
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                if (writeBytes == -1)
                {
                    perror("Error writing student info to client!");
                    return false;
                }
                readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            }
            
        }
    }

    close(courseFileDescriptor);
    bzero(writeBuffer, sizeof(writeBuffer));
    
    strcpy(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing student info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    return true;

}

// Locking Critical Section =====================================================================================================================
bool lock_critical_section(int semIdentifier, struct sembuf *semOp)
{
    semOp->sem_flg = SEM_UNDO;
    semOp->sem_op = -1;
    semOp->sem_num = 0;
    int semopStatus = semop(semIdentifier, semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while locking critical section");
        return false;
    }
    return true;
}

// Unlocking Critical Section =====================================================================================================================
bool unlock_critical_section(int semIdentifier, struct sembuf *semOp)
{
    semOp->sem_op = 1;
    int semopStatus = semop(semIdentifier, semOp, 1);
    if (semopStatus == -1)
    {
        perror("Error while operating on semaphore!");
        _exit(1);
    }
    return true;
}


// =====================================================================================================================

#endif
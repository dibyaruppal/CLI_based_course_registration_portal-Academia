#ifndef STUDENT_FUNCTIONS
#define STUDENT_FUNCTIONS

// Semaphores are necessary joint account due the design choice I've made
#include <sys/ipc.h>
#include <sys/sem.h>

#include "./common.h"

struct Student loggedInStudent;
int semIdentifier;

// Function Prototypes =================================

bool student_operation_handler(int connFD);
bool enroll_courses(int connFD, int studentID);
bool unenroll_courses(int connFD, int studentID);
bool get_student_course_details(int connFD, int studentID);
bool change_password(int connFD);

// =====================================================

// Function Definition =================================

bool student_operation_handler(int connFD)
{
    if (login_handler(false, false, connFD, &loggedInStudent, NULL))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1024], writeBuffer[1024]; // A buffer used for reading & writing to the client

        // Get a semaphore for the student
        key_t semKey = ftok(STUDENT_FILE, loggedInStudent.id); // Generate a key based on the account number hence, different customers will have different semaphores

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
        strcpy(writeBuffer, STUDENT_LOGIN_SUCCESS);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, STUDENT_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing STUDENT_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading student's choice for STUDENT_MENU");
                return false;
            }
            
            
            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                enroll_courses(connFD, loggedInStudent.id);
                break;
            case 2:
                unenroll_courses(connFD, loggedInStudent.id);
                break;
            case 3:
                get_student_course_details(connFD, loggedInStudent.id);
                break;
            case 4:
                get_student_details(connFD, loggedInStudent.id);
                break;
            case 5:
                change_password(connFD);
                break;
            default:
                writeBytes = write(connFD, STUDENT_LOGOUT, strlen(STUDENT_LOGOUT));
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

//Enroll Course =====================================================================================================================
bool enroll_courses(int connFD, int studentID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Course course;
    struct Student student;

    off_t offset;
    int lockingStatus;

    get_all_course_details(connFD);

    // Get id from user
    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, OPTION_MENU);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing OPTION_MENU message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student age response from client!");
        return false;
    }

    int ID = atoi(readBuffer);
    if (ID < 0)
    {
        // Either client has sent age as 0 (which is invalid) or has entered a non-numeric string
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    // Adding Course id into Student record
    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }

    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};


    // Lock the record to be read
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    close(studentFileDescriptor);

    // Checking Course ID already present or not
    for(int i=0;i<student.number_of_enrolled_course;i++)
    {
        if(student.enrolled_course[i]==ID)
        {
            // Course record doesn't exist
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, COURSE_ALREADY_ENROLLED);
            strcat(writeBuffer, "^");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing COURSE_ALREADY_ENROLLED message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
    }

    int n_course = student.number_of_enrolled_course;
    if(n_course < MAX_COURSE_ENROLLED)
    {
        student.enrolled_course[n_course] = ID;
        student.number_of_enrolled_course = n_course + 1;
    }
    else{
        writeBytes = write(connFD, MAX_COURSE_ENROLLED_REACHED, strlen(MAX_COURSE_ENROLLED_REACHED));
        if (writeBytes == -1)
        {
            perror("Error writing MAX_COURSE_ENROLLED_REACHED message to client!");
            return false;
        }
        return false;
    }

    studentFileDescriptor = open(STUDENT_FILE, O_WRONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }
    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on student record!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while writing update student info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLKW, &lock);

    close(studentFileDescriptor);

    // Updated Alloted seat for that Course id

    int courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1)
    {
        perror("Error while opening course file");
        return false;
    }
    else
    {
        int offset = lseek(courseFileDescriptor, ID * sizeof(struct Course), SEEK_SET);
        if (errno == EINVAL)
        {
            // Course record doesn't exist
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
        else if (offset == -1)
        {
            perror("Error seeking to course record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
        int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on course record!");
            return false;
        }

        readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
        if (readBytes == -1)
        {
            perror("Error while reading Student record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(courseFileDescriptor, F_SETLK, &lock);

        close(courseFileDescriptor);

        course.enrolled_students_id[course.alloted_seat] = studentID;
        course.alloted_seat = course.alloted_seat + 1;
        
        courseFileDescriptor = open(COURSE_FILE, O_WRONLY, S_IRWXU);
        if (courseFileDescriptor == -1)
        {
            perror("Error while creating / opening course file!");
            return false;
        }
        offset = lseek(courseFileDescriptor, ID * sizeof(struct Course), SEEK_SET);
        if (offset == -1)
        {
            perror("Error while seeking to required course record!");
            return false;
        }
        lock.l_type = F_WRLCK;
        lock.l_start = offset;
        lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error while obtaining write lock on faculty record!");
            return false;
        }

        writeBytes = write(courseFileDescriptor, &course, sizeof(struct Course));
        if (writeBytes == -1)
        {
            perror("Error while writing course record to file!");
            return false;
        }

        // Unlock the record
        lock.l_type = F_UNLCK;
        fcntl(courseFileDescriptor, F_SETLK, &lock);

        close(courseFileDescriptor);
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d", STUDENT_ENROLL_COURSE_SUCCESS, ID);
    strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(read)); // Dummy read

    return true;
}


//Unenroll Course =====================================================================================================================
bool unenroll_courses(int connFD, int studentID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Course course;
    struct Student student;

    off_t offset;
    int lockingStatus;
    int courseID;

    bool enrollementStatus = get_student_course_details(connFD, studentID);
    if(enrollementStatus == false)
    {
        return false;
    }
    // Unenroll Course from the Course 
    writeBytes = write(connFD, STUDENT_UNENROLL_COURSE, strlen(STUDENT_UNENROLL_COURSE));
    if (writeBytes == -1)
    {
        perror("Error while writing STUDENT_UNENROLL_COURSE message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading course ID from client!");
        return false;
    }

    courseID = atoi(readBuffer);

    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }
    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
        
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    struct flock lock_c = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};
    // Lock the record to be read
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock_c);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock_c.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock_c);

    close(studentFileDescriptor);

    // Checking Course ID present or not
    int course_exist = 0;
    for(int i=0;i<student.number_of_enrolled_course;i++)
    {
        if(student.enrolled_course[i] == courseID)
        {
            course_exist = 1;
            break;
        }
    }
    if(course_exist == 0)
    {
        // Course record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, THIS_COURSE_NOT_ENROLLED);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing THIS_COURSE_NOT_ENROLLED message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    // Removing course ID from student structure
    studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }

    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};


    // Lock the record to be read
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    close(studentFileDescriptor);

    // Search the Position of the Course ID
    int pos=0;
    int n_course = student.number_of_enrolled_course;
    for(int j=0; j<n_course; j++)
    {
        if(student.enrolled_course[j] == courseID)
        {
            pos = j;
            break;
        }
    }
    /* Copy next element value to current element */
    for(int i=pos; i<n_course; i++)
    {
        student.enrolled_course[i] = student.enrolled_course[i + 1];
    }

    /* Decrement number_of_enrolled_course by 1 */
    student.number_of_enrolled_course = n_course -1;

    studentFileDescriptor = open(STUDENT_FILE, O_WRONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }
    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on student record!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &student, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while writing update student info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLKW, &lock);

    close(studentFileDescriptor);

    // Remove student ID from course structure
    // Updated Alloted seat for that Course id

    int courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1)
    {
        perror("Error while opening course file");
        return false;
    }
    else
    {
        int offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
        if (errno == EINVAL)
        {
            // Course record doesn't exist
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
        else if (offset == -1)
        {
            perror("Error seeking to course record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
        int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on course record!");
            return false;
        }

        readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
        if (readBytes == -1)
        {
            perror("Error while reading Student record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(courseFileDescriptor, F_SETLK, &lock);

        close(courseFileDescriptor);
        
        // Search the Position of the Course ID
        int pos=0;
        int n_alloted_seat = course.alloted_seat;
        for(int j=0; j<n_alloted_seat; j++)
        {
            if(course.enrolled_students_id[j] == studentID)
            {
                pos = j;
                break;
            }
        }

        // Delete the course ID from the offer_course_id array
        for(int i=pos; i<n_alloted_seat-1; i++)
        {
                course.enrolled_students_id[i] = course.enrolled_students_id[i + 1];
        }
        /* Decrement alloted_seat by 1 */
        course.alloted_seat = n_alloted_seat - 1;

        courseFileDescriptor = open(COURSE_FILE, O_WRONLY, S_IRWXU);
        if (courseFileDescriptor == -1)
        {
            perror("Error while creating / opening course file!");
            return false;
        }
        offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
        if (offset == -1)
        {
            perror("Error while seeking to required course record!");
            return false;
        }
        lock.l_type = F_WRLCK;
        lock.l_start = offset;
        lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error while obtaining write lock on faculty record!");
            return false;
        }

        writeBytes = write(courseFileDescriptor, &course, sizeof(struct Course));
        if (writeBytes == -1)
        {
            perror("Error while writing course record to file!");
            return false;
        }

        // Unlock the record
        lock.l_type = F_UNLCK;
        fcntl(courseFileDescriptor, F_SETLK, &lock);

        close(courseFileDescriptor);
    }

    return true;
}

//Fetching enroll course details =====================================================================================================================
bool get_student_course_details(int connFD, int studentID)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Course course;
    struct Student student;

    off_t offset;
    int lockingStatus;

    // Get enrolled course id from Student
    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1)
    {
        perror("Error while opening student file");
        return false;
    }

    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
    
    if (offset == -1)
    {
        perror("Error while seeking to required student record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};


    // Lock the record to be read
    lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(studentFileDescriptor, &student, sizeof(struct Student));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(studentFileDescriptor, F_SETLK, &lock);

    close(studentFileDescriptor);

    if(student.number_of_enrolled_course == 0)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, COURSE_NOT_ENROLLED);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing COURSE_NOT_ENROLLED message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else
    {
        int courseFileDescriptor;
        struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Course), getpid()};

        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "Your Enrolled Course details : \n^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing enrollment info to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

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
        for(int i=0;i<student.number_of_enrolled_course;i++)
        {
            int cid = student.enrolled_course[i];
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
                sprintf(writeBuffer, "\t-Course ID : %d\tCourse Name : %s^", course.course_id, course.course_name);
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                if (writeBytes == -1)
                {
                    perror("Error writing student info to client!");
                    return false;
                }
                readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            }
            
        }
        close(courseFileDescriptor);
    }

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




//Change Password =====================================================================================================================
bool change_password(int connFD)
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
        perror("Error writing PASSWORD_CHANGE_OLD_PASS message to student!");
        unlock_critical_section(semIdentifier,&semOp);
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading old password response from student");
        unlock_critical_section(semIdentifier,&semOp);
        return false;
    }

    //if (strcmp(crypt(readBuffer, SALT_BAE), loggedInCustomer.password) == 0)
    if (strcmp(readBuffer, loggedInStudent.password) == 0)
    {
        // Password matches with old password
        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS, strlen(PASSWORD_CHANGE_NEW_PASS));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS message to student!");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading new password response from student");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }

        // strcpy(newPassword, crypt(readBuffer, SALT_BAE));
        strcpy(newPassword, readBuffer);

        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_RE, strlen(PASSWORD_CHANGE_NEW_PASS_RE));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS_RE message to student!");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading new password reenter response from student");
            unlock_critical_section(semIdentifier,&semOp);
            return false;
        }

        // if (strcmp(crypt(readBuffer, SALT_BAE), newPassword) == 0)
        if (strcmp(readBuffer, newPassword) == 0)
        {
            // New & reentered passwords match

            strcpy(loggedInStudent.password, newPassword);

            int studentFileDescriptor = open(STUDENT_FILE, O_WRONLY);
            if (studentFileDescriptor == -1)
            {
                perror("Error opening customer file!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            off_t offset = lseek(studentFileDescriptor, loggedInStudent.id * sizeof(struct Student), SEEK_SET);
            if (offset == -1)
            {
                perror("Error seeking to the customer record!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};
            int lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining write lock on student record!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            writeBytes = write(studentFileDescriptor, &loggedInStudent, sizeof(struct Student));
            if (writeBytes == -1)
            {
                perror("Error storing updated student password into student record!");
                unlock_critical_section(semIdentifier,&semOp);
                return false;
            }

            lock.l_type = F_UNLCK;
            lockingStatus = fcntl(studentFileDescriptor, F_SETLK, &lock);

            close(studentFileDescriptor);

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
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
bool add_courses(int connFD, int facultyID);
bool remove_offered_courses(int connFD, int facultyID);
bool view_enrollments_in_course(int connFD, int facultyID);
bool view_offered_course(int connFD, int facultyID);
bool change_password_faculty(int connFD);

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
                add_courses(connFD, loggedInFaculty.id);
                break;
            case 2:
                remove_offered_courses(connFD, loggedInFaculty.id);
                break;
            case 3:
                view_enrollments_in_course(connFD, loggedInFaculty.id);
                break;
            case 4:
                view_offered_course(connFD, loggedInFaculty.id);
                break;
            case 5:
                get_faculty_details(connFD, loggedInFaculty.id);
                break;
            case 6:
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

//ADD Courses =====================================================================================================================
bool add_courses(int connFD, int facultyID){
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Course newCourse, prevCourse;
    struct Faculty faculty;

    // Adding Course into Course record
    int courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1 && errno == ENOENT)
    {
        // Course file was never created
        newCourse.course_id = 0;
    }
    else if (courseFileDescriptor == -1)
    {
        perror("Error while opening course file");
        return false;
    }
    else
    {
        int offset = lseek(courseFileDescriptor, -sizeof(struct Course), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last course record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
        int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on course record!");
            return false;
        }

        readBytes = read(courseFileDescriptor, &prevCourse, sizeof(struct Course));
        if (readBytes == -1)
        {
            perror("Error while reading Student record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(courseFileDescriptor, F_SETLK, &lock);

        close(courseFileDescriptor);

        newCourse.course_id = prevCourse.course_id + 1;
    }
    writeBytes = write(connFD, FACULTY_ADD_COURSE_NAME, strlen(FACULTY_ADD_COURSE_NAME));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_COURSE_NAME message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading course name response from client!");
        return false;
    }

    strcpy(newCourse.course_name, readBuffer);

    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, FACULTY_ADD_TOTAL_SEAT);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing FACULTY_ADD_TOTAL_SEAT message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading total seat of the course response from client!");
        return false;
    }

    int totalSeat = atoi(readBuffer);
    if (totalSeat < 0)
    {
        // Either client has sent seat number as >0 (which is invalid) or has entered a non-numeric string
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
    newCourse.seat = totalSeat;
    newCourse.alloted_seat = 0;
    newCourse.active = 1;

    courseFileDescriptor = open(COURSE_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (courseFileDescriptor == -1)
    {
        perror("Error while creating / opening course file!");
        return false;
    }

    writeBytes = write(courseFileDescriptor, &newCourse, sizeof(struct Course));
    if (writeBytes == -1)
    {
        perror("Error while writing course record to file!");
        return false;
    }

    close(courseFileDescriptor);

    // Adding Course id into faculty record
    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }

    off_t offset;
    int lockingStatus;

    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
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
        perror("Error while seeking to required customer record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock);

    close(facultyFileDescriptor);

    int n_course = faculty.number_of_course;
    if(n_course < MAX_COURSE_OFFERED)
    {
        faculty.offer_course_id[n_course] = newCourse.course_id;
        faculty.number_of_course = n_course + 1;
    }
    else{
        writeBytes = write(connFD, MAX_COURSE_OFFERING_REACHED, strlen(MAX_COURSE_OFFERING_REACHED));
        if (writeBytes == -1)
        {
            perror("Error writing MAX_COURSE_OFFERING_REACHED message to client!");
            return false;
        }
        return false;
    }

    facultyFileDescriptor = open(FACULTY_FILE, O_WRONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }
    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required faculty record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on faculty record!");
        return false;
    }

    writeBytes = write(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (writeBytes == -1)
    {
        perror("Error while writing update faculty info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLKW, &lock);

    close(facultyFileDescriptor);


    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d", FACULTY_ADD_COURSE_SUCCESS, newCourse.course_id);
    strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(read)); // Dummy read
    return true;
}

//Remove Courses =====================================================================================================================
bool remove_offered_courses(int connFD, int facultyID){
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Course course;
    struct Faculty faculty;

    int courseID;

    off_t offset;
    int lockingStatus;

    // Deactivating Course ID from the Course record
    writeBytes = write(connFD, FACULTY_DEACTIVATE_COURSE, strlen(FACULTY_DEACTIVATE_COURSE));
    if (writeBytes == -1)
    {
        perror("Error while writing FACULTY_DEACTIVATE_COURSE message to client!");
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

    int courseFileDescriptor = open(COURSE_FILE, O_RDONLY);
    if (courseFileDescriptor == -1)
    {
        // course File doesn't exist
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

    offset = lseek(courseFileDescriptor, courseID * sizeof(struct Course), SEEK_SET);
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
        perror("Error while seeking to required course record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on course record!");
        return false;
    }

    readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
    if (readBytes == -1)
    {
        perror("Error while reading course record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLK, &lock);

    close(courseFileDescriptor);

    course.active = 0;

    courseFileDescriptor = open(COURSE_FILE, O_WRONLY);
    if (courseFileDescriptor == -1)
    {
        perror("Error while opening course file");
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
        perror("Error while obtaining write lock on course record!");
        return false;
    }

    writeBytes = write(courseFileDescriptor, &course, sizeof(struct Course));
    if (writeBytes == -1)
    {
        perror("Error while writing update course info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(courseFileDescriptor, F_SETLKW, &lock);

    close(courseFileDescriptor);


    // Deleting Course id from faculty record
    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }

    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
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
        perror("Error while seeking to required admin record!");
        return false;
    }

    struct flock lock_faculty = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock_faculty);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on faculty record!");
        return false;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (readBytes == -1)
    {
        perror("Error while reading faculty record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock_faculty);

    close(facultyFileDescriptor);

    // Search the Position of the Course ID
    int pos;
    int n_course = faculty.number_of_course;
    for(int j=0; j<n_course; j++)
    {
        if(faculty.offer_course_id[j] == courseID)
        {
            pos = j;
            break;
        }
    }

    // Delete the course ID from the offer_course_id array
    for(int i=pos; i<n_course-1; i++)
    {
            faculty.offer_course_id[i] = faculty.offer_course_id[i + 1];
    }
    /* Decrement number_of_course by 1 */
    faculty.number_of_course = n_course - 1;
    

    facultyFileDescriptor = open(FACULTY_FILE, O_WRONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }
    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required faculty record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on faculty record!");
        return false;
    }

    writeBytes = write(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (writeBytes == -1)
    {
        perror("Error while writing update faculty info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLKW, &lock);

    close(facultyFileDescriptor);



    writeBytes = write(connFD, FACULTY_DEACTIVATE_COURSE_SUCCESS, strlen(FACULTY_DEACTIVATE_COURSE_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing FACULTY_DEACTIVATE_COURSE_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}


//Fetching student enrollment Details =====================================================================================================================
bool view_enrollments_in_course(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1024], writeBuffer[1024]; // A buffer for reading from / writing to the socket
    char tempBuffer[1024];

    struct Course course;
    struct Faculty faculty;

    int courseFileDescriptor;

    //Reading Faculty Record to get offer_course_id array
    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }

    off_t offset;
    int lockingStatus;

    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
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
        perror("Error while seeking to required customer record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock);

    close(facultyFileDescriptor);
    if(faculty.number_of_course == 0)
    {
        writeBytes = write(connFD, NO_COURSE_OFFERED, strlen(NO_COURSE_OFFERED));
        if (writeBytes == -1)
        {
            perror("Error while writing NO_COURSE_OFFERED message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;    
    }

    if (faculty.number_of_course == 0)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "No Course Added by You Yet\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing enrollment info to client!");
            return false;
        }
    }
    
    // readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    //Looping throuhg each id and get details from Course record
    
    else
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "Enrollment details of Students : \n^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing enrollment info to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

        for(int i=0;i<faculty.number_of_course;i++)
        {
            int courseID = faculty.offer_course_id[i];
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
                perror("Error while seeking to required course record!");
                return false;
            }

            struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
            // lock.l_start = offset;

            int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error while obtaining read lock on the student file!");
                return false;
            }

            readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
            if (readBytes == -1)
            {
                perror("Error reading course record from file!");
                return false;
            }

            lock.l_type = F_UNLCK;
            fcntl(courseFileDescriptor, F_SETLK, &lock);

            bzero(writeBuffer, sizeof(writeBuffer));
            sprintf(writeBuffer, "\t- Course ID : %d,\tCourse Name : %s,\tenrolled student : %d,\tEmpty seats : %d^", course.course_id, course.course_name, course.alloted_seat, course.seat-course.alloted_seat);
            close(courseFileDescriptor);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error writing course info to client!");
                return false;
            }

            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        }
    }
    
    bzero(writeBuffer, sizeof(writeBuffer));
    writeBytes = write(connFD, RETURN_TO_MAIN_MENU, strlen(RETURN_TO_MAIN_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing RETURN_TO_MAIN_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    return true;
}


//Fetching offered course Details =====================================================================================================================
bool view_offered_course(int connFD, int facultyID)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1024], writeBuffer[1024]; // A buffer for reading from / writing to the socket
    char tempBuffer[1024];

    struct Course course;
    struct Faculty faculty;

    int courseFileDescriptor;

    //Reading Faculty Record to get offer_course_id array
    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening faculty file");
        return false;
    }

    off_t offset;
    int lockingStatus;

    offset = lseek(facultyFileDescriptor, facultyID * sizeof(struct Faculty), SEEK_SET);
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
        perror("Error while seeking to required customer record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};

    // Lock the record to be read
    lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on student record!");
        return false;
    }

    readBytes = read(facultyFileDescriptor, &faculty, sizeof(struct Faculty));
    if (readBytes == -1)
    {
        perror("Error while reading student record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLK, &lock);

    close(facultyFileDescriptor);
    if(faculty.number_of_course == 0)
    {
        writeBytes = write(connFD, NO_COURSE_OFFERED, strlen(NO_COURSE_OFFERED));
        if (writeBytes == -1)
        {
            perror("Error while writing NO_COURSE_OFFERED message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;    
    }

    if (faculty.number_of_course == 0)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "No Course Added by You Yet\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing enrollment info to client!");
            return false;
        }
    }
    
    // readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    //Looping throuhg each id and get details from Course record
    
    else
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        sprintf(writeBuffer, "Your offered Course details : \n^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error writing enrollment info to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

        for(int i=0;i<faculty.number_of_course;i++)
        {
            int courseID = faculty.offer_course_id[i];
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
                perror("Error while seeking to required course record!");
                return false;
            }

            struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
            // lock.l_start = offset;

            int lockingStatus = fcntl(courseFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error while obtaining read lock on the student file!");
                return false;
            }

            readBytes = read(courseFileDescriptor, &course, sizeof(struct Course));
            if (readBytes == -1)
            {
                perror("Error reading course record from file!");
                return false;
            }

            lock.l_type = F_UNLCK;
            fcntl(courseFileDescriptor, F_SETLK, &lock);

            bzero(writeBuffer, sizeof(writeBuffer));
            sprintf(writeBuffer, "\t- Course ID : %d,\tCourse Name : %s^", course.course_id, course.course_name);
            close(courseFileDescriptor);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error writing course info to client!");
                return false;
            }

            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        }
    }
    
    bzero(writeBuffer, sizeof(writeBuffer));
    writeBytes = write(connFD, RETURN_TO_MAIN_MENU, strlen(RETURN_TO_MAIN_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing RETURN_TO_MAIN_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
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
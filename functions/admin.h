#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

#include "./common.h"

// Function Prototypes =================================

bool admin_operation_handler(int connFD);
bool add_student(int connFD);
bool add_faculty(int connFD);
bool activate_student(int connFD);
bool deactivate_student(int connFD);
bool update_student_details(int connFD);
bool update_faculty_details(int connFD);

// =====================================================

// Function Definition =================================

// =====================================================

bool admin_operation_handler(int connFD)
{

    if (login_handler(true, false, connFD, NULL, NULL))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1024], writeBuffer[1024]; // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ADMIN_LOGIN_SUCCESS);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, ADMIN_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ADMIN_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for ADMIN_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            case 1:
                add_student(connFD);
                break;
            case 2:
                add_faculty(connFD);
                break;
            case 3: 
                activate_student(connFD);
                break;
            case 4:
                deactivate_student(connFD);
                break;
            case 5:
                update_student_details(connFD);   
                break;
            case 6:
                update_faculty_details(connFD);
                break;
            case 7:
                get_student_details(connFD, -1);
                break;
            case 8:
                get_faculty_details(connFD, -1);
                break;
            default:
                writeBytes = write(connFD, ADMIN_LOGOUT, strlen(ADMIN_LOGOUT));
                return false;
            }
        }
    }
    else
    {
        // ADMIN LOGIN FAILED
        return false;
    }
    return true;
}

// For Student -------------------------------------------------------------------------
bool add_student(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Student newStudent, prevStudent;

    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
    if (studentFileDescriptor == -1 && errno == ENOENT)
    {
        // Student file was never created
        newStudent.id = 0;
    }
    else if (studentFileDescriptor == -1)
    {
        perror("Error while opening account file");
        return false;
    }
    else
    {
        int offset = lseek(studentFileDescriptor, -sizeof(struct Student), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Account record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};
        int lockingStatus = fcntl(studentFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Student record!");
            return false;
        }

        readBytes = read(studentFileDescriptor, &prevStudent, sizeof(struct Student));
        if (readBytes == -1)
        {
            perror("Error while reading Student record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(studentFileDescriptor, F_SETLK, &lock);

        close(studentFileDescriptor);

        newStudent.id = prevStudent.id + 1;
    }
    writeBytes = write(connFD, ADMIN_ADD_STUDENT_NAME, strlen(ADMIN_ADD_STUDENT_NAME));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_NAME message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student name response from client!");
        return false;
    }

    strcpy(newStudent.name, readBuffer);

    writeBytes = write(connFD, ADMIN_ADD_STUDENT_GENDER, strlen(ADMIN_ADD_STUDENT_GENDER));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_GENDER message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student gender response from client!");
        return false;
    }

    if (readBuffer[0] == 'M' || readBuffer[0] == 'F' || readBuffer[0] == 'O')
        newStudent.gender = readBuffer[0];
    else
    {
        writeBytes = write(connFD, ADMIN_ADD_STUDENT_WRONG_GENDER, strlen(ADMIN_ADD_STUDENT_WRONG_GENDER));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    strcpy(writeBuffer, ADMIN_ADD_STUDENT_AGE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_AGE message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student age response from client!");
        return false;
    }

    int studentAge = atoi(readBuffer);
    if (studentAge == 0)
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
    newStudent.age = studentAge;

    writeBytes = write(connFD, ADMIN_ADD_STUDENT_DEPT, strlen(ADMIN_ADD_STUDENT_DEPT));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_STUDENT_DEPT message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student department response from client!");
        return false;
    }

    const char *str_cse = "CSE";
    const char *str_ece = "ECE";

    if (strcmp(str_cse,readBuffer) == 0 || strcmp(str_ece,readBuffer) == 0)
        strcpy(newStudent.dept, readBuffer);
    else
    {
        writeBytes = write(connFD, ADMIN_ADD_STUDENT_WRONG_DEPT, strlen(ADMIN_ADD_STUDENT_WRONG_DEPT));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    //Setting login ID for student added by Admin
    bzero(readBuffer, sizeof(readBuffer));
    strcpy(newStudent.login, newStudent.name);
    strcat(newStudent.login, "-");
    sprintf(writeBuffer, "%d", newStudent.id);
    strcat(newStudent.login, writeBuffer);

    //Setting Default Password for student added by Admin
    strcpy(newStudent.password, newStudent.name);

    // Setting Activated staus to the Student
    newStudent.active = 1;

    studentFileDescriptor = open(STUDENT_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (studentFileDescriptor == -1)
    {
        perror("Error while creating / opening student file!");
        return false;
    }

    writeBytes = write(studentFileDescriptor, &newStudent, sizeof(struct Student));
    if (writeBytes == -1)
    {
        perror("Error while writing Account record to file!");
        return false;
    }

    close(studentFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d", ADMIN_ADD_STUDENT_SUCCESS, newStudent.id);
    strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(read)); // Dummy read
    return true;

}

// For Faculty -------------------------------------------------------------------------
bool add_faculty(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Faculty newFaculty, prevFaculty;

    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1 && errno == ENOENT)
    {
        // Faculty file was never created
        newFaculty.id = 0;
    }
    else if (facultyFileDescriptor == -1)
    {
        perror("Error while opening account file");
        return false;
    }
    else
    {
        int offset = lseek(facultyFileDescriptor, -sizeof(struct Faculty), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Account record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};
        int lockingStatus = fcntl(facultyFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on Faculty record!");
            return false;
        }

        readBytes = read(facultyFileDescriptor, &prevFaculty, sizeof(struct Faculty));
        if (readBytes == -1)
        {
            perror("Error while reading Faculty record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(facultyFileDescriptor, F_SETLK, &lock);

        close(facultyFileDescriptor);

        newFaculty.id = prevFaculty.id + 1;
    }
    writeBytes = write(connFD, ADMIN_ADD_FACULTY_NAME, strlen(ADMIN_ADD_FACULTY_NAME));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_NAME message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading student name response from client!");
        return false;
    }

    strcpy(newFaculty.name, readBuffer);

    writeBytes = write(connFD, ADMIN_ADD_FACULTY_DEPT, strlen(ADMIN_ADD_FACULTY_DEPT));
    if (writeBytes == -1)
    {
        perror("Error writing ADMIN_ADD_FACULTY_DEPT message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading faculty department response from client!");
        return false;
    }

    const char *str_cse = "CSE";
    const char *str_ece = "ECE";

    if (strcmp(readBuffer,str_cse) == 0 || strcmp(readBuffer,str_ece) == 0)
        strcpy(newFaculty.dept, readBuffer);
    else
    {
        writeBytes = write(connFD, ADMIN_ADD_FACULTY_WRONG_DEPT, strlen(ADMIN_ADD_FACULTY_WRONG_DEPT));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    //Setting login ID for student added by Admin
    bzero(readBuffer, sizeof(readBuffer));
    strcpy(newFaculty.login, newFaculty.name);
    strcat(newFaculty.login, "-");
    sprintf(writeBuffer, "%d", newFaculty.id);
    strcat(newFaculty.login, writeBuffer);

    //Setting Default Password for student added by Admin
    strcpy(newFaculty.password, newFaculty.name);

    facultyFileDescriptor = open(FACULTY_FILE, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while creating / opening faculty file!");
        return false;
    }

    writeBytes = write(facultyFileDescriptor, &newFaculty, sizeof(struct Faculty));
    if (writeBytes == -1)
    {
        perror("Error while writing Faculty record to file!");
        return false;
    }

    close(facultyFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d", ADMIN_ADD_FACULTY_SUCCESS, newFaculty.id);
    strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(read)); // Dummy read
    return true;

}

// For Activating Student -------------------------------------------------------------------------
bool activate_student(int connFD){
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Student student;

    int studentID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_ACTIVATE_STUDENT, strlen(ADMIN_ACTIVATE_STUDENT));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_ACTIVATE_STUDENT message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading student ID from client!");
        return false;
    }

    studentID = atoi(readBuffer);

    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
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

    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
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
        perror("Error while seeking to required customer record!");
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

    student.active = 1;

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

    writeBytes = write(connFD, ADMIN_ACTIVATE_STUDENT_SUCCESS, strlen(ADMIN_ACTIVATE_STUDENT_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_ACTIVATE_STUDENT_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;

}

// For Deactivating Student -------------------------------------------------------------------------
bool deactivate_student(int connFD){
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Student student;

    int studentID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_DEACTIVATE_STUDENT, strlen(ADMIN_DEACTIVATE_STUDENT));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_DEACTIVATE_STUDENT message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading student ID from client!");
        return false;
    }

    studentID = atoi(readBuffer);

    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
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

    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
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
        perror("Error while seeking to required customer record!");
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

    student.active = 0;

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

    writeBytes = write(connFD, ADMIN_DEACTIVATE_STUDENT_SUCCESS, strlen(ADMIN_DEACTIVATE_STUDENT_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_DEACTIVATE_STUDENT_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;

}


// For Updating Student Details -------------------------------------------------------------------------
bool update_student_details(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Student student;

    int studentID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_ID, strlen(ADMIN_UPDATE_STUDENT_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_STUDENT_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading student ID from client!");
        return false;
    }

    studentID = atoi(readBuffer);

    int studentFileDescriptor = open(STUDENT_FILE, O_RDONLY);
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
    
    offset = lseek(studentFileDescriptor, studentID * sizeof(struct Student), SEEK_SET);
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

    writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_MENU, strlen(ADMIN_UPDATE_STUDENT_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_STUDENT_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting customer modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    if (choice == 0)
    { // A non-numeric string was passed to atoi
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

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_NEW_NAME, strlen(ADMIN_UPDATE_STUDENT_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_NEW_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new name from client!");
            return false;
        }
        strcpy(student.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_NEW_AGE, strlen(ADMIN_UPDATE_STUDENT_NEW_AGE));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_NEW_AGE message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new age from client!");
            return false;
        }
        int updatedAge = atoi(readBuffer);
        if (updatedAge == 0)
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
        student.age = updatedAge;
        break;
    case 3:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_NEW_GENDER, strlen(ADMIN_UPDATE_STUDENT_NEW_GENDER));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_NEW_GENDER message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new gender from client!");
            return false;
        }
        student.gender = readBuffer[0];
        break;
    case 4:
        writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_NEW_DEPT, strlen(ADMIN_UPDATE_STUDENT_NEW_DEPT));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_STUDENT_NEW_DEPT message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for student's new department from client!");
            return false;
        }
        const char *str_cse = "CSE";
        const char *str_ece = "ECE";

        if (strcmp(str_cse,readBuffer) == 0 || strcmp(str_ece,readBuffer) == 0)
            strcpy(student.dept, readBuffer);
        else
        {
            writeBytes = write(connFD, ADMIN_ADD_STUDENT_WRONG_DEPT, strlen(ADMIN_ADD_STUDENT_WRONG_DEPT));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        
        break;
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, INVALID_MENU_CHOICE);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing INVALID_MENU_CHOICE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
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

    writeBytes = write(connFD, ADMIN_UPDATE_STUDENT_SUCCESS, strlen(ADMIN_UPDATE_STUDENT_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_STUDENT_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}

// For Updating Faculty Details -------------------------------------------------------------------------
bool update_faculty_details(int connFD)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1024], writeBuffer[1024];

    struct Faculty faculty;

    int facultyID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_ID, strlen(ADMIN_UPDATE_FACULTY_ID));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_FACULTY_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while reading student ID from client!");
        return false;
    }

    facultyID = atoi(readBuffer);

    int facultyFileDescriptor = open(FACULTY_FILE, O_RDONLY);
    if (facultyFileDescriptor == -1)
    {
        // Faculty File doesn't exist
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
        perror("Error while seeking to required faculty record!");
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

    writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_MENU, strlen(ADMIN_UPDATE_FACULTY_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_FACULTY_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting faculty modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    if (choice == 0)
    { // A non-numeric string was passed to atoi
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

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_NEW_NAME, strlen(ADMIN_UPDATE_FACULTY_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_FACULTY_NEW_NAME message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new name from client!");
            return false;
        }
        strcpy(faculty.name, readBuffer);
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_NEW_DEPT, strlen(ADMIN_UPDATE_FACULTY_NEW_DEPT));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_UPDATE_FACULTY_NEW_DEPT message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for faculty's new department from client!");
            return false;
        }
        const char *str_cse = "CSE";
        const char *str_ece = "ECE";

        if (strcmp(str_cse,readBuffer) == 0 || strcmp(str_ece,readBuffer) == 0)
            strcpy(faculty.dept, readBuffer);
        else
        {
            writeBytes = write(connFD, ADMIN_ADD_FACULTY_WRONG_DEPT, strlen(ADMIN_ADD_FACULTY_WRONG_DEPT));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        
        break;
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, INVALID_MENU_CHOICE);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing INVALID_MENU_CHOICE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    facultyFileDescriptor = open(FACULTY_FILE, O_WRONLY);
    if (facultyFileDescriptor == -1)
    {
        perror("Error while opening student file");
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
        perror("Error while writing update student info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(facultyFileDescriptor, F_SETLKW, &lock);

    close(facultyFileDescriptor);

    writeBytes = write(connFD, ADMIN_UPDATE_FACULTY_SUCCESS, strlen(ADMIN_UPDATE_FACULTY_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_UPDATE_FACULTY_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}
// =====================================================

#endif
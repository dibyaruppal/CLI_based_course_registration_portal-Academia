# CLI_based_course_registration_portal-Academia
The project aims to develop a Academia Portal that is user-friendly and multifunctional. The project mainly deals with concurrent course management operations in a safe and secure environment.

## Features of ERP:
1. Handling multiple connections simultaneously.
2. Socket programming is used to implement the client-server model.
3. TCP connection for effective and reliable communication.
4. Storage/Database: Flat file system is used.
5. Concurrency: File Locking is used.
6. System calls related to process management, file management, file locking and inter-process communication mechanisms is used.
7. Password-protected login system for admin, faculty and student.

## Functionalities
### Admin Module:
After successful login of admin, can perform the following operations:

1. Add Student
2. Add Faculty
3. Activate Student
4. Deactivate Student
5. Update Student Details
6. Update Faculty Details
7. View Student details
8. View Faculty details
9. Exit

### Faculty Module:
After successful login of faculty, can perform the following operations:


1. Add New course
2. Remove offered Course
3. View Enrollments in course
4. Modify Course Details
5. View offered Courses
6. View personal Account details
7. Change Password
8. Exit

### Student Module:
After successful login of student, can perform the following operations:

View All Offering Courses
1. Enroll New course
2. Unenroll from already enrolled Courses
3. View Enrolled Course
4. View personal Account details
5. Change Password
6. Exit


## Setup and Execution
First clone the repository 
```
    git clone https://github.com/rahulbollisetty/SS-Miniproject.git
```
> The client-server socket uses 9020 port number
> Make sure that no other processs is using that number

Run the server by running the following on your command line:
```
    ./server
```
Run the client by running the following on your command line:
```
    ./client
```
> The server can handle multiple clients, so you can execute client code simultaneously on different terminals

- Admin default credentials:
  - Login-Id: `admin`
  - Password: `Admin123`

- Faculty credentials format:
  - Faculty Login-Id Format: `Name-{faculty-Id.}`
  - Faculty Password Format: `Name`
 
- Student credentials format:
  - Student Login-Id Format: `Name-{student-Id.}`
  - Student Password Format: `Name`

<br/><br/>

Dibyarup Pal\
MTech CSE [MT2023090]\
International Institute of Information Technology, Bangalore.

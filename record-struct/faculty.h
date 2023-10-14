#ifndef FACULTY_RECORD
#define FACULTY_RECORD

struct Faculty
{
    int id; // 0, 1, 2 ....
    char name[25];
    char dept[4]; // Department Name
    // structCourse *offer_course[3];
    int number_of_course;
    int offer_course_id[10];
    // Login Credentials
    char login[30]; // Format : name-id (name will the first word in the structure member `name`)
    char password[30];
};

#endif
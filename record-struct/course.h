#ifndef COURSE_RECORD
#define COURSE_RECORD

struct Course
{
    int course_id; // 0, 1, 2 ....
    char course_name[25]; // Course Name
    int seat; // Total number on seat
    int alloted_seat; // Total number of filled seat
    int enrolled_students_id[200];
    int active;
};

#endif
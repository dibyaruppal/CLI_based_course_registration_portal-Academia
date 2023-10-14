#ifndef SERVER_CONSTANTS
#define SERVER_CONSTANTS


#define INITIAL_PROMPT "........................................Welcome to Academia :: Course Registration POrtal......................................\nWho are you?\n1. Admin\t2. Faculty\t3. Student \nPress any other number to exit\nEnter the number corresponding to the choice! : "

// ========== COMMON TEXT =============================

// LOGIN
#define LOGIN_ID "Enter your login ID"
#define PASSWORD "Enter your password \n# "
#define INVALID_LOGIN "The login ID specified doesn't exist!$"
#define INVALID_PASSWORD "The password specified doesn't match!$"
// #define SALT_BAE "666"

// GET ACCOUNT DETAILS
// #define GET_ACCOUNT_NUMBER "Enter the account number of the account you're searching for"
// #define GET_CUSTOMER_ID "Enter the customer ID of the customer you're searching for"

#define ERRON_INPUT_FOR_NUMBER "It seems you have passed a sequence of alphabets when a number was expected or you have entered an invalid number!\nYou'll now be redirected to the main menu!^"

#define INVALID_MENU_CHOICE "It seems you've made an invalid menu choice\nYou'll now be redirected to the main menu!^"

//#define CUSTOMER_ID_DOESNT_EXIT "No customer could be found for the given ID"
//#define CUSTOMER_LOGIN_ID_DOESNT_EXIT "No customer could be found for the given login ID$"

#define STUDENT_ID_DOESNT_EXIT "No student could be found for the given ID"
#define STUDENT_LOGIN_ID_DOESNT_EXIT "No student could be found for the given login ID$"

// #define ACCOUNT_ID_DOESNT_EXIT "No account could be found for the given account number"

// #define TRANSACTIONS_NOT_FOUND "No transactions were performed on this account by the customer!^"

// GET STUDENT DETAILS
#define GET_STUDENT_ID "Enter the STUDENT ID of the student you're searching for"
#define STUDENT_ID_DOESNT_EXIT "No student could be found for the given ID"
#define STUDENT_LOGIN_ID_DOESNT_EXIT "No student could be found for the given login ID$"

#define ERRON_INPUT_FOR_NUMBER "It seems you have passed a sequence of alphabets when a number was expected or you have entered an invalid number!\nYou'll now be redirected to the main menu!^"

#define INVALID_MENU_CHOICE "It seems you've made an invalid menu choice\nYou'll now be redirected to the main menu!^"

// GET FACULTY DETAILS
#define GET_FACULTY_ID "Enter the FACULTY ID of the student you're searching for"
#define FACULTY_ID_DOESNT_EXIT "No faculty could be found for the given ID"
#define FACULTY_LOGIN_ID_DOESNT_EXIT "No FACULTY could be found for the given login ID$"

// // GET FACULTY DETAILS
// #define GET_FACULTY_ID "Enter the FACULTY ID of the student you're searching for"
// #define FACULTY_ID_DOESNT_EXIT "No faculty could be found for the given ID"
// #define FACULTY_LOGIN_ID_DOESNT_EXIT "No FACULTY could be found for the given login ID$"

///////////////

// #define ACCOUNT_ID_DOESNT_EXIT "No account could be found for the given account number"

// #define TRANSACTIONS_NOT_FOUND "No transactions were performed on this account by the customer!^"

// ====================================================

// ========== ADMIN SPECIFIC TEXT======================

// LOGIN WELCOME
#define ADMIN_LOGIN_WELCOME "Welcome dear admin! With great power comes great responsibility!\nEnter your credentials to unlock this power!"
#define ADMIN_LOGIN_SUCCESS "Welcome superman!"

// ADMIN MENU
#define ADMIN_MENU "1. Add Student\n2. Add Faculty\n3. Activate Student\n4. Deactivate Student\n5. Update Student details\n6. Update Faculty details\n7. Get Student Details\n8. Get Faculty Details\nPress any other key to logout"

// ADD STUDENT
// #define ADMIN_ADD_STUDENT "Enter the details for the primary customer\n"
// #define ADMIN_UPDATE_STUDENT "What is the customer's age?"
// #define ADMIN_ADD_CUSTOMER_AUTOGEN_LOGIN "The autogenerated login ID for the customer is : "
// #define AUTOGEN_PASSWORD "spookytime" // append to end of next string macro
// #define ADMIN_ADD_CUSTOMER_AUTOGEN_PASSWORD "The autogenerated password for the customer is : "


#define ADMIN_ADD_STUDENT_NAME "Enter the Name of the Student\n"
#define ADMIN_ADD_STUDENT_GENDER "Enter the Gender (M: Male, F: Female, O: Other)\n"
#define ADMIN_ADD_STUDENT_AGE "Enter the age of the Student\n"
#define ADMIN_ADD_STUDENT_DEPT "Enter the Department of the Student( CSE or ECE )\n"
#define ADMIN_ADD_STUDENT_WRONG_GENDER "It seems you've enter a wrong gender choice!\nYou'll now be redirected to the main menu!^"
#define ADMIN_ADD_STUDENT_SUCCESS "The student account is created succesfully : "
#define ADMIN_ADD_STUDENT_WRONG_DEPT "It seems you've enter a wrong Department choice!\nYou'll now be redirected to the main menu!^"

// ADD FACULTY
#define ADMIN_ADD_FACULTY_NAME "Enter the Name of the Faculty\n"
#define ADMIN_ADD_FACULTY_DEPT "Enter the Department of the Faculty( CSE or ECE )\n"
#define ADMIN_ADD_FACULTY_SUCCESS "The Faculty account is created succesfully : "
#define ADMIN_ADD_FACULTY_WRONG_DEPT "It seems you've enter a wrong Department choice!\nYou'll now be redirected to the main menu!^"

// DELETE ACCOUNT
// #define ADMIN_DEL_ACCOUNT_NO "What is the account number of the account you want to delete?"
// #define ADMIN_DEL_ACCOUNT_SUCCESS "This account has been successfully deleted\nRedirecting you to the main menu ...^"
// #define ADMIN_DEL_ACCOUNT_FAILURE "This account cannot be deleted since it still has some money\nRedirecting you to the main menu ...^"

// MODIFY STUDENT INFO
#define ADMIN_UPDATE_STUDENT_ID "Enter the ID of the student who's information you want to edit"
#define ADMIN_UPDATE_STUDENT_MENU "Which information would you like to modify?\n1. Name    2. Age    3. Gender    4. Department\nPress any other key to cancel"
#define ADMIN_UPDATE_STUDENT_NEW_NAME "What's the updated value for name?"
#define ADMIN_UPDATE_STUDENT_NEW_GENDER "What's the updated value for gender?"
#define ADMIN_UPDATE_STUDENT_NEW_AGE "What's the updated value for age?"
#define ADMIN_UPDATE_STUDENT_NEW_DEPT "What's the updated value for Department?"
#define ADMIN_ACTIVATE_STUDENT "Enter the ID of the student whom you want to activate : "
#define ADMIN_DEACTIVATE_STUDENT "Enter the ID of the student whom you want to deactivate"
#define ADMIN_DEACTIVATE_STUDENT_SUCCESS "The metioned student is deactivated successfully !!\nYou'll now be redirected to the main menu!^"
#define ADMIN_ACTIVATE_STUDENT_SUCCESS "The metioned student is activated successfully !!\nYou'll now be redirected to the main menu!^"

#define ADMIN_UPDATE_STUDENT_SUCCESS "The required modification was successfully made!\nYou'll now be redirected to the main menu!^"

// MODIFY FACULTY INFO
#define ADMIN_UPDATE_FACULTY_ID "Enter the ID of the faculty who's information you want to edit"
#define ADMIN_UPDATE_FACULTY_MENU "Which information would you like to modify?\n1. Name    2. Department \nPress any other key to cancel"
#define ADMIN_UPDATE_FACULTY_NEW_NAME "What's the updated value for name?"
#define ADMIN_UPDATE_FACULTY_NEW_DEPT "What's the updated value for Department?"

#define ADMIN_UPDATE_FACULTY_SUCCESS "The required modification was successfully made!\nYou'll now be redirected to the main menu!^"

#define ADMIN_LOGOUT "Logging you out now superman! Good bye!$"


// ENROLL COURSE
#define OPTION_MENU "Enter the ID of the course from above in which you want to enroll"
#define MAX_COURSE_ENROLLED 10
#define MAX_COURSE_ENROLLED_REACHED "Maximum number of Course Enrolled Reached by faculty\n^"
#define STUDENT_ENROLL_COURSE_SUCCESS "The course is added succesfully : "
#define COURSE_NOT_ENROLLED "No Course is enrolled at this moment"
#define COURSE_ALREADY_ENROLLED "You have already enrolled in this course"
#define THIS_COURSE_NOT_ENROLLED "You have not enrolled in this course"

// UNENROLL COURSE
#define STUDENT_UNENROLL_COURSE "Enter the ID of the course from which you want to unenroll from the above listed Courses"
// #define OPTION_MENU "Enter the ID of the course from above in which you want to enroll"
// #define MAX_COURSE_ENROLLED 10
// #define MAX_COURSE_ENROLLED_REACHED "Maximum number of Course Enrolled Reached by faculty\n^"
// #define STUDENT_ENROLL_COURSE_SUCCESS "The course is added succesfully : "
// #define COURSE_NOT_ENROLLED "No Course is enrolled at this moment"
// #define COURSE_ALREADY_ENROLLED "You have already enrolled in this course"
// #define COURSE_NOT_ENROLLED "You have not enrolled in this course"

// ====================================================

// ========== STUDENT SPECIFIC TEXT===================

// LOGIN WELCOME
#define STUDENT_LOGIN_WELCOME "Welcome dear student! Enter your credentials to gain access to your Academia account!"
#define STUDENT_LOGIN_SUCCESS "Welcome beloved student!"

#define STUDENT_LOGOUT "Logging you out now dear student! Good bye!$"

#define FACULTY_LOGIN_WELCOME "Welcome dear Faculty! Enter your credentials to gain access to your Academia account!"
#define FACULTY_LOGIN_SUCCESS "Welcome beloved Faculty!"


// STUDENT MENU
#define STUDENT_MENU "1. Enroll new Course\n2. Unenroll from already Enrolled Course\n3. View Enrolled Courses\n4. Get your Account Details\n5. Change Password\nPress any other key to logout"

#define STUDENT_ACCOUNT_DEACTIVATED "It seems your account has been deactivated!\nPlease contact Admin to activate it!$"

// ========== FACULTY SPECIFIC TEXT===================

// LOGIN WELCOME
#define FACULTY_LOGIN_WELCOME "Welcome dear Faculty! Enter your credentials to gain access to your Academia account!"
#define FACULTY_LOGIN_SUCCESS "Welcome beloved Faculty!"

#define FACULTY_LOGOUT "Logging you out now dear Faculty! Good bye!$"

// FACULTY MENU
#define FACULTY_MENU "1. Add new Course\n2. Remove offerd Course\n3. View Enrollment in Course\n4. View Your offered courses \n5. Get your Account Details\n6. Change Password\nPress any other key to logout"



// #define DEPOSIT_AMOUNT "How much is it that you want to add into your bank?"
// #define DEPOSIT_AMOUNT_INVALID "You seem to have passed an invalid amount!^"
// #define DEPOSIT_AMOUNT_SUCCESS "The specified amount has been successfully added to your bank account!^"

// #define WITHDRAW_AMOUNT "How much is it that you want to withdraw from your bank?"
// #define WITHDRAW_AMOUNT_INVALID "You seem to have either passed an invalid amount or you don't have enough money in your bank to withdraw the specified amount^"
// #define WITHDRAW_AMOUNT_SUCCESS "The specified amount has been successfully withdrawn from your bank account!^"



// CHANGE PASSWORD
#define PASSWORD_CHANGE_OLD_PASS "Enter your old password"
#define PASSWORD_CHANGE_OLD_PASS_INVALID "The entered password doesn't seem to match with the old password"
#define PASSWORD_CHANGE_NEW_PASS "Enter the new password"
#define PASSWORD_CHANGE_NEW_PASS_RE "Reenter the new password"
#define PASSWORD_CHANGE_NEW_PASS_INVALID "The new password and the reentered passwords doesn't match!^"
#define PASSWORD_CHANGE_SUCCESS "Password successfully changed!^"


//
#define SEG_FAULT_ID_DOESNT_EXIT "Login ID Doesn't Exist !!$"

// ADD COURSE
#define FACULTY_ADD_COURSE_NAME "Enter the Name of the Course\n"
#define FACULTY_ADD_COURSE_SUCCESS "The course is added succesfully : "
#define FACULTY_ADD_TOTAL_SEAT "Enter the total number of seats of the Course\n"
#define FACULTY_DEACTIVATE_COURSE "Enter the ID of the course which you want to remove"
#define FACULTY_DEACTIVATE_COURSE_SUCCESS "The metioned course is removed successfully !!\nYou'll now be redirected to the main menu!^"

#define COURSE_ID_DOESNT_EXIT "No course could be found for the given ID"
#define NO_COURSE_OFFERED "No course is offered by the given Faculty^"

#define MAX_COURSE_OFFERED 10
#define MAX_COURSE_OFFERING_REACHED "Maximum number of Course Offering Reached by faculty\n^"
#define COURSE_NOT_AVAILABLE "No Course is being offered at this moment"

#define RETURN_TO_MAIN_MENU "\nYou'll now be redirected to the main menu!^"
// #define ADMIN_ADD_STUDENT_GENDER "Enter the Gender (M: Male, F: Female, O: Other)\n"
// #define ADMIN_ADD_STUDENT_DEPT "Enter the Department of the Student( CSE or ECE )\n"
// #define ADMIN_ADD_STUDENT_WRONG_GENDER "It seems you've enter a wrong gender choice!\nYou'll now be redirected to the main menu!^"

// #define ADMIN_ADD_STUDENT_WRONG_DEPT "It seems you've enter a wrong Department choice!\nYou'll now be redirected to the main menu!^"
// ====================================================


#define FACULTY_FILE "./records/faculty.txt"
#define STUDENT_FILE "./records/student.txt"
#define COURSE_FILE "./records/course.txt"
#endif
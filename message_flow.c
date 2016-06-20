#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include "Utilities.h"
#include "message_flow.h"
#include "LinkedList.h"


#define NMMAX 30
#define MAX_PREFERENCE_INPUT 3
#define NUM_OF_PREFERENCES 5



int communication_flow(int *communication_progress, ThreadArg *args, size_t size, char *response_message,
                       int *existing_user, char *filepath) {
    int option = 0;
    int status = 0;

    char *path_to_users = strcat(filepath, choose_label("users_path"));

    switch (*communication_progress) {
        case 0:
            if (search_in_list(response_message, NULL) != NULL) {
                send_message(args->socket, choose_label("already_logged_user"));
                return 0;
            }
            *existing_user = login_part(args, size, response_message, path_to_users);
            (++*communication_progress);
            break;
        case 1:
            status = password_part(args, existing_user, response_message, path_to_users, choose_label("main_menu"));

            (status == -1) ? *communication_progress = 0 : ++(*communication_progress);
            break;
        case 2:
            option = response_message[0] - '0';

            main_menu_flow(option, args, path_to_users, filepath);

            break;
        default:
            send_message(args->socket, choose_label("invalid_input"));
    }

    return 0;

}


void main_menu_flow(int option, ThreadArg *args, char *path_to_users, char *filepath) {
    switch (option) {
        case 1:
            update_profile(args, path_to_users);
            send_message(args->socket, choose_label("main_menu"));
            break;
        case 2:
            send_message(args->socket, choose_label("search_user_intro"));
            see_other_profiles_func(args->socket, args->login, filepath);
            break;
        case 3:
            send_message(args->socket, choose_label("update_profile"));
            find_matching_profile(args->socket, args->login, filepath);
            send_message(args->socket, choose_label("main_menu"));
            break;
        case 4:
            send_message(args->socket, choose_label("ask_a_date"));
            ask_for_a_date(args->socket, filepath, args->login);
            send_message(args->socket, choose_label("main_menu"));
            break;
        case 5:
            break;

        default:
            send_message(args->socket, choose_label("invalid_input"));
    }
}


char *ask_for_a_date(int socket, char *filepath, char *login) {
    char result[NMMAX];
    char my_dir[strlen(filepath) + strlen(choose_label("pending")) + strlen(login) + 1];
    char her_dir[strlen(filepath) + strlen(choose_label("pending")) + NMMAX + 1];
    memset(my_dir, 0, sizeof(my_dir));
    memset(her_dir, 0, sizeof(her_dir));
    struct stat s;

    snprintf(my_dir, sizeof(my_dir), "%s%s%s", filepath, login, choose_label("pending"));

    int size = 0;
    ThreadArg *invited_one;

    while (true) {
        if ((size = (int) recive_message(socket, result)) == 0)
            break;
        result[size - 2] = '\0';
        strip(result);

        snprintf(her_dir, sizeof(her_dir), "%s%s", filepath, result);
        if (stat(her_dir, &s) == -1) {
            if (errno == ENOENT)
                return NULL;
        }
        strcat(her_dir, choose_label("pending"));

        if ((invited_one = search_in_list(result, NULL)) != NULL) {
            pthread_mutex_lock(&invited_one->lock);
            send_message(invited_one->socket, "Go on a date with me\n");
            send_message(invited_one->socket, login);
            pthread_mutex_unlock(&invited_one->lock);
        }

        lock_and_write(strcat(result, ";"), my_dir, "a");
        lock_and_write(strcat(login, ";"), her_dir, "a");

        break;
    }

    return NULL;
}

void find_matching_profile(int socket, char *login, char *filepath) {


    char properties[strlen(filepath) + strlen(choose_label("properties_path")) + strlen(login) + 1];
    memset(properties, 0, sizeof(properties));

    snprintf(properties, sizeof(properties), "%s%s%s", filepath, login, choose_label("properties_path"));

    char *directories[NMMAX];
    char result[NMMAX];
    int size = 0;
    int number_choosen = 0;

    char *his_position = calloc(1, NMMAX);
    int num_of_users = 0;


    if ((size = (int) recive_message(socket, result)) == 0)
        return;
    result[size - 2] = '\0';
    strip(result);

    if (strlen(result) < 2)
        return;

    number_choosen = result[0] - '0';


    strncpy(his_position, result, sizeof(result));
    his_position[sizeof(result)] = '\0';
    his_position++;


    num_of_users = listdir(filepath, login, 1, directories);
    directories[num_of_users] = NULL;
    find_in_other_users(socket, num_of_users, his_position, number_choosen, filepath, directories);
}


void find_in_other_users(int socket, int num_of_users, char *my_attribute, int chosen_attr, char *filepath,
                         char *directories[]) {
    int i = 0;

    char tempArr[num_of_users][NMMAX];

    int filesize = 0;


    for (int l = 0; l < num_of_users; l++) {
        strncpy(tempArr[l], directories[l], strlen(directories[l]));
        tempArr[l][strlen(directories[l])] = '\0';
    }

    char *his_file_content = NULL;
    char properties[strlen(filepath) + strlen(choose_label("properties_path")) + NMMAX];
    memset(properties, 0, sizeof(properties));
    char *temp;
    char *temp2;


    for (; i < num_of_users; i++) {
        filesize = 0;
        memset(properties, 0, sizeof(properties));


        snprintf(properties, sizeof(properties), "%s%s%s", filepath, tempArr[i], choose_label("properties_path"));
        temp = lock_and_read(his_file_content, properties, &filesize);
        temp2 = read_specific_attribute_from_string(chosen_attr, filesize, temp);
        free(temp);
        if (temp2[0] != '\0' && strcmp(my_attribute, temp2) == 0) {
            send_message(socket, strcat(tempArr[i], " - "));
        }
    }


}


char *read_specific_attribute_from_string(int number, int filesize, char *file_content) {
    int flag = 0;
    char *result = (char *) calloc(1, MAX_PREFERENCE_INPUT);
    int j = 0;
    for (int i = 0; i < filesize; i++) {
        if (file_content[i] == ';')
            number--;

        while (file_content[++i] != ';' && number == 0) {
            flag = 1;
            if (file_content[i] != ' ')
                result[j++] = file_content[i];
        }
        if (flag == 1)
            return result;

    }

    return NULL;
}

void see_other_profiles_func(int socket, char *login, char *filepath) {
    int number_choosen = 0;


    char *directories[NMMAX];
    char result[NMMAX];
    int size = 0;


    while (true) {
        if ((size = (int) recive_message(socket, result)) == 0)
            break;
        result[size - 2] = '\0';
        strip(result);


        number_choosen = result[0] - '0';

        switch (number_choosen) {
            case 0:
                send_message(socket, choose_label("main_menu"));
                return;
            case 1:
                search_by_login(socket, result, filepath);
                break;
            case 2:
                search_one_by_one(socket, result, login, filepath, directories);
                break;
            default:
                send_message(socket, choose_label("invalid_input"));
                continue;
        }

    }


}


void search_one_by_one(int socket, char *message, char *login, char *filepath, char *allUsersLogins[]) {
    int num_of_users = 0;

    char *result = NULL;

    if (strlen(message) > 1) {
        send_message(socket, choose_label("invalid_input"));
        return;
    }
    num_of_users = listdir(filepath, " ", 0, allUsersLogins);

    allUsersLogins[num_of_users] = NULL;
    int length_of_all_users = 0;

    for (int i = 0; i < num_of_users; i++)
        length_of_all_users += strlen(allUsersLogins[i]);


    if (display_chosen_user(socket, result, num_of_users, filepath) == 0)
        return;


}

int display_chosen_user(int socket, char *result, int num_of_users, char *filepath) {
    char *allUsersLogins[NMMAX];
    int count_user_directory = 0;
    char response_message[NMMAX];
    int user_count = 0;
    size_t size_messagge = 0;
    while (true) {

        send_message(socket, choose_label("next_user"));
        if ((size_messagge = recive_message(socket, response_message)) == 0)
            break;

        response_message[size_messagge - 2] = '\0';

        if (strlen(response_message) > 1 || strlen(response_message) == 0) {
            send_message(socket, choose_label("invalid_input"));
            continue;
        }

        result = next_exit_user_search(response_message[0], ++count_user_directory, filepath);

        if (result == NULL) {
            send_message(socket, choose_label("search_user_intro"));
            return 0;
        }
        num_of_users = listdir(filepath, " ", 0, allUsersLogins);
        allUsersLogins[num_of_users] = NULL;
        send_message(socket, allUsersLogins[user_count % num_of_users]);
        send_message(socket, show_users_avatar(filepath, allUsersLogins[user_count % num_of_users]));
        user_count++;
        send_message(socket, result);

        free(result);
    }
    return 1;
}


char *next_exit_user_search(char option, int countUser, char *filepath) {
    char *allUsersLogins[NMMAX];
    int num_of_users = listdir(filepath, " ", 0, allUsersLogins);
    allUsersLogins[num_of_users] = NULL;

    char temparr[NMMAX];
    memset(temparr, 0, sizeof(temparr));
    switch (option) {
        case 'E':
        case 'e':
            return NULL;
        case 'N':
        case 'n':

            snprintf(temparr, strlen(allUsersLogins[(countUser - 1) % num_of_users]) + 1, "%s",
                     allUsersLogins[(countUser - 1) % num_of_users]);
            return choose_specific_user_from_list(temparr, filepath);
    }
    return NULL;
}


void search_by_login(int socket, char *message, char *filepath) {
    char *allUserNames[NMMAX];
    int flag = 0;
    char choosen_name[NMMAX];
    char *ascii_image = NULL;
    char *result = NULL;
    int num_of_users = 0;
    if (strlen(message) <= 1) {
        send_message(socket, choose_label("invalid_input"));
        return;
    }
    memcpy(choosen_name, &message[1], strlen(message));
    num_of_users = listdir(filepath, " ", 0, allUserNames);
    allUserNames[num_of_users] = NULL;

    for (int i = 0; i < num_of_users; i++) {
        if (strcmp(choosen_name, allUserNames[i]) == 0) {

            ascii_image = show_users_avatar(filepath, allUserNames[i]);
            send_message(socket, ascii_image);
            free(ascii_image);
            num_of_users = listdir(filepath, " ", 0, allUserNames);
            allUserNames[num_of_users] = NULL;
            result = choose_specific_user_from_list(allUserNames[i], filepath);
            send_message(socket, result);
            send_message(socket, choose_label("search_user_intro"));
            free(result);
            flag = 1;
            break;
        }
    }


    if (flag == 0)
        send_message(socket, choose_label("no_such_user"));
}


char *show_users_avatar(char *filepath, char *chosen_name) {
    char *image = NULL;
    char *ascii = "/asci";
    char acsii_path[strlen(filepath) + strlen(chosen_name) + strlen(ascii)];
    int filesize = 0;
    snprintf(acsii_path, strlen(filepath) + strlen(chosen_name) + strlen(ascii) + 1, "%s%s%s", filepath, chosen_name,
             ascii);
    return lock_and_read(image, acsii_path, &filesize);
}

char *choose_specific_user_from_list(const char chosen_user[], char *filepath) {
    int length_of_elements = 0;

    char *labels[5];
    labels[0] = "Sex: ";
    labels[1] = "Weight: ";
    labels[2] = "Age: ";
    labels[3] = "Hair Color: ";
    labels[4] = "Eye Color: ";


    int size_of_labels = 0;

    char properties[strlen(filepath) + strlen(choose_label("properties_path")) + strlen(chosen_user) + 1];
    memset(properties, 0, sizeof(properties));
    char *user_prop[NUM_OF_PREFERENCES];
    for (int i = 0; i < NUM_OF_PREFERENCES; i++) {
        if ((user_prop[i] = (char *) malloc(NMMAX)) == NULL)
            ERR("malloc");
        size_of_labels += strlen(labels[i]);
    }
    snprintf(properties, sizeof(properties), "%s%s%s", filepath, chosen_user, choose_label("properties_path"));
    int j = 0;
    int filesize = 0;
    char *file_content = NULL;

    file_content = lock_and_read(file_content, properties, &filesize);

    int counter = 0;
    for (int i = 0; i < filesize && counter < NUM_OF_PREFERENCES; i++) {
        while (file_content[++i] != ';') {
            if (file_content[i] != ' ')
                user_prop[counter][j++] = file_content[i];
        }
        user_prop[counter][j] = '\0';
        length_of_elements += strlen(user_prop[counter]);
        j = 0;
        counter++;
        --i;

    }

    free(file_content);
    file_content = NULL;

    return concat_2d_array_to_string(user_prop, length_of_elements, NUM_OF_PREFERENCES, labels, size_of_labels);


}

void update_profile(ThreadArg *args, char *filepath) {
    char letter_chosen = 0;
    char result[NMMAX];
    int number_choosen = 0;
    int numerical_walues = 0;
    enum options_enum {
        zero, sex, weight, age, hair_length, eye
    };

    int size = 0;


    send_message(args->socket, choose_label("update_profile"));

    while (true) {
        if ((size = (int) recive_message(args->socket, result)) == 0)
            break;

        result[size - 2] = '\0';
        strip(result);


        number_choosen = result[0] - '0';

        switch (number_choosen) {
            case zero:
                return;
            case weight:
            case age:
                numerical_walues = sublist_number(result);
                break;
            case hair_length:
            case sex:
            case eye:
                letter_chosen = sublist_letter(result);
                break;
            default:
                send_message(args->socket, choose_label("invalid_input"));
                break;


        }

        update_specific_characteristics(args, number_choosen, letter_chosen, numerical_walues, filepath,
                                        choose_label("invalid_input"));

    }


}

int update_specific_characteristics(ThreadArg *args, int number_choosen, int letter_chosen, int numerical_walues,
                                    char *filepath, char *options_after_wrong_input) {
    enum options_enum {
        zero, sex, weight, age, hair_length, eye
    };
    switch (number_choosen) {
        case sex:

            if (fill_position_char(letter_chosen, 'm', 'M', 'f', 'F')) {

                find_and_replace_in_file(args, filepath, sex, letter_chosen, -1);
                send_message(args->socket, choose_label("update_profile"));
            }
            else
                send_message(args->socket, options_after_wrong_input);
            break;
        case weight:
            if (fill_position_number(numerical_walues, 30, 250)) {
                find_and_replace_in_file(args, filepath, weight, 0, numerical_walues);
                send_message(args->socket, choose_label("update_profile"));
            }
            else
                send_message(args->socket, options_after_wrong_input);
            break;
        case age:
            if (fill_position_number(numerical_walues, 18, 99)) {
                find_and_replace_in_file(args, filepath, age, 0, numerical_walues);
                send_message(args->socket, choose_label("update_profile"));
            }
            else
                send_message(args->socket, options_after_wrong_input);
            break;
        case hair_length:
            if (fill_position_char(letter_chosen, 'l', 'L', 's', 'S')) {
                find_and_replace_in_file(args, filepath, hair_length, letter_chosen, -1);
                send_message(args->socket, choose_label("update_profile"));
            }
            else
                send_message(args->socket, options_after_wrong_input);
            break;
        case eye:
            if (fill_position_char(letter_chosen, 'b', 'B', 'g', 'G')) {
                find_and_replace_in_file(args, filepath, eye, letter_chosen, -1);
                send_message(args->socket, choose_label("update_profile"));
            }
            else
                send_message(args->socket, options_after_wrong_input);
            break;


        default:
            return -1;


    }

    return 0;
}


void find_and_replace_in_file(ThreadArg *args, char *filepath, int position_to_replace, char toreplace_letter,
                              int toreplace_number) {
    char numtostr[MAX_PREFERENCE_INPUT];
    char *property_path = choose_label("properties_path");
    char *user_login = args->login;
    int filesize = 0;
    char full_properties_path[strlen(filepath) + strlen(property_path) + strlen(user_login) + 1];
    snprintf(full_properties_path, sizeof(full_properties_path), "%s%s%s", filepath, user_login, property_path);

    if (toreplace_number != -1) {
        sprintf(numtostr, "%d", toreplace_number);
    }

    char *file_cont = NULL;
    file_cont = lock_and_read(file_cont, full_properties_path, &filesize);


    (toreplace_number != -1) ?
    replace_old_characteristics_with_number(filesize, file_cont, position_to_replace, numtostr)
                             : replace_old_characteristics_letter(filesize, file_cont, position_to_replace,
                                                                  toreplace_letter);

    lock_and_write(file_cont, full_properties_path, "w+");

    file_cont = NULL;


}


void replace_old_characteristics_with_number(int filesize, char *file_content, int number, char *num_to_string) {
    int j = 0;
    int counter = 0;
    for (int i = 0; i < filesize; i++) {
        if (file_content[i] != ';')
            continue;
        counter++;
        if (counter != number)
            continue;

        while (j < MAX_PREFERENCE_INPUT) {
            file_content[i + j + 1] = (char) ((num_to_string[j] != '\0') ?
                                              num_to_string[j] : ' ');
            j++;
        }
        return;

    }
}


void replace_old_characteristics_letter(int filesize, char *file_content, int number, char toreplace_letter) {
    int counter = 0;
    for (int i = 0; i < filesize; i++) {
        if (file_content[i] != ';')
            continue;
        counter++;

        if (counter == number) {
            file_content[1 + i] = toreplace_letter;
            return;
        }

    }
}


int login_part(ThreadArg *args, size_t size, char *response_message, char *path_to_users) {
    char *ask_for_password = "Write your password: \n";
    int if_user_exists = 0;
    char *path_to_specific_user;

    if ((args->login = (char *) malloc(sizeof(size - 2))) == NULL)
        perror("Malloc:");

    strcpy(args->login, response_message);


    create_dir(path_to_users);


    path_to_specific_user = strcat(path_to_users, args->login);
    if_user_exists = create_dir(path_to_specific_user);

    send_message(args->socket, ask_for_password);


    return if_user_exists;
}


int password_part(ThreadArg *args, int *existing_user, char *response_message, char *pathcurr, char *options) {
    char *tempPath;
    FILE *pFile = NULL;
    int output_value;
    struct stat t;
    int filesize = 0;
    strcat(pathcurr, args->login);
    char *pendinginfo = NULL;

    if ((tempPath = (char *) malloc(strlen(pathcurr))) == NULL)
        perror("Malloc:");

    strcpy(tempPath, pathcurr);
    output_value = (*existing_user == 0) ?
                   new_user(args, pathcurr, options, response_message)
                                         : user_exists(args, pFile, response_message, options, tempPath);


    if (stat(strcat(tempPath, choose_label("pending")), &t) == -1) {
        if (errno == ENOENT)
            return output_value;
    }
    else {
        pendinginfo = lock_and_read(pendinginfo, tempPath, &filesize);
        send_message(args->socket, "You have been invited by: ");
        send_message(args->socket, pendinginfo);
        free(pendinginfo);
    }

    free(tempPath);


    return output_value;
}

int user_exists(ThreadArg *args, FILE *pFile, char *response_message, char *options, char *pathcurr) {
    char *wrong_password = "Wrong password \nWrite your login:\n";
    char *password = choose_label("password_path");

    char path_password[strlen(pathcurr) + strlen(password) + 1];
    snprintf(path_password, sizeof(path_password), "%s%s", pathcurr, password);


    char *existing_password = NULL;
    int filesize = 0;

    existing_password = lock_and_read(existing_password, path_password, &filesize);


    if (strcmp(response_message, existing_password) == 0) {


        send_message(args->socket, options);

        free(existing_password);
        return 1;
    }
    else {
        send_message(args->socket, wrong_password);
        pthread_mutex_lock(&args->lock);
        strcpy(args->login, "");
        pthread_mutex_unlock(&args->lock);
        free(existing_password);
        return -1;
    }

}


int new_user(ThreadArg *args, char *pathcurr, char *options, char *password) {
    char *password_end_path = "/password.txt";
    char *property_path = "/properties.txt";
    char *ascii_path = "ASCII_ART/";
    char *local_ascii_file = "/ascii";
    char outputAscii[strlen(pathcurr) + strlen(args->login) + strlen(local_ascii_file) + 1];
    char ascii_part[strlen(args->filepath) + strlen(ascii_path) + 1];
    char properties[strlen(pathcurr) + strlen(property_path) + 1];
    char password_path[strlen(pathcurr) + strlen(password_end_path) + 1];
    int r = 0;
    int filesize = 0;
    char *ascii_image = NULL;
    snprintf(properties, sizeof(properties), "%s%s", pathcurr, property_path);
    snprintf(password_path, sizeof(password_path), "%s%s", pathcurr, password_end_path);


    snprintf(outputAscii, strlen(pathcurr) + strlen(local_ascii_file), "%s%s", pathcurr, local_ascii_file);

    srand(getpid());
    r = rand() % 5 + 1;
    snprintf(ascii_part, strlen(args->filepath) + strlen(ascii_path) + sizeof(int), "%s%s%d", args->filepath,
             ascii_path, r);

    ascii_image = lock_and_read(ascii_image, ascii_part, &filesize);
    lock_and_write(ascii_image, outputAscii, "w+");
    free(ascii_image);

    lock_and_write(password, password_path, "w+");
    lock_and_write(";   ;   ;   ;   ;   ;", properties, "w+");
    send_message(args->socket, options);
    return 0;

}


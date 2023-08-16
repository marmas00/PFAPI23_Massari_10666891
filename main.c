#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 8192


//declaring types

typedef enum boolean {true,false} bool;
typedef enum {left_to_right, right_to_left} direction;
typedef enum {add, delete} mode;

typedef struct car {

    int autonomy;
    struct car* next;

} car_t;

typedef struct node {
    int dist;
    struct node* next;
    struct node* prev;
    struct node* parent;
    bool visited;
    bool in_queue;
    bool in_temp_queue;
    int number_of_cars;
    int max_reach;
    int last_modified_id;

    car_t* car_park;
} station_node_t;

typedef struct pile {

    station_node_t* queued_node;
    struct pile* next;

} pile_t;

//pointer to the end of the queue

pile_t* tail_node;

//id of the current operation

int this_operation_id = 0;

//station management functions

//inserts node in order

bool insert_new_station(station_node_t** head, station_node_t* newnode){


    station_node_t* current;
    station_node_t* previous;
    previous = NULL;
    current = *head;

    while (current != NULL && newnode->dist >= current->dist) {
        if (current->dist == newnode->dist) return false;
        previous = current;
        current = current->next;

    }

    newnode->next = current;
    newnode->prev = previous;

    if (previous != NULL)
        newnode->prev->next = newnode;
    else
        *head = newnode;

    if (current != NULL)
        newnode->next->prev = newnode;

    return true;

}


//create new station node

station_node_t* create_new_node (int new_dist){

    station_node_t* new_node = (station_node_t*) malloc(sizeof (station_node_t));


    new_node->dist = new_dist;
    new_node->visited = false;
    new_node->in_queue = false;
    new_node->in_temp_queue = false;
    new_node->number_of_cars = 0;
    new_node->parent = NULL;
    new_node->max_reach = 0;
    new_node->car_park = NULL;
    new_node->last_modified_id = -1;


    return new_node;

}

//search for a node given the dist

station_node_t * search_node(station_node_t* head, int searched_dist){

    station_node_t* temp = head;
    while (temp != NULL){
        if (temp->dist == searched_dist) {
            return temp;
        }
        if (temp->dist > searched_dist) return NULL;
        temp=temp->next;
    }
    return NULL;

}



//queue management functions


//pushes a node on the head of the queue

void head_enqueue(pile_t** head, station_node_t* nodeptr){

    pile_t* newnode = (pile_t*) malloc (sizeof(pile_t));
    newnode->queued_node=nodeptr;
    newnode->next= *head;
    *head=newnode;

}

//pushes a node on the tail of the queue

void tail_enqueue(pile_t** tail, station_node_t* nodeptr){

    pile_t* newnode = malloc (sizeof(pile_t));
    newnode->next = NULL;
    newnode->queued_node = nodeptr;

    if ((*tail) == NULL ){

        *tail = newnode;
        tail_node = newnode;
        tail_node->next = NULL;

    }
    else {

        tail_node->next = newnode;
        tail_node = newnode;

    }

}

//pops a node in front of the queue

station_node_t* dequeue(pile_t** head){


    if ( *head == NULL ) return NULL;

    pile_t* head_ref = *head;
    station_node_t* dequeued_node = head_ref->queued_node;
    ( *head ) = ( *head ) -> next;
    free( head_ref );

    return dequeued_node;

}

//checks if queue is empty

int isempty(pile_t* head){

    if (head == NULL) return 1;
    else return 0;

}

//prints queue

void print_queue (pile_t** head){
    pile_t* current;
    current = *head;
    while(current != NULL){
        printf("%d", current->queued_node->dist);
        current=current->next;
        if (current==NULL) break;
        printf(" ");
    }
    printf("\n");

}

//used to reset "visited" flags to false after ending a shortest path operation

void reset_visited_flags (pile_t** head){


    pile_t* current = *head;
    while(current != NULL){
        current->queued_node->visited=false;
        current=current->next;
    }


}

//used to reset "in_temp_queue" flags to false and to set "in_queue" flags to true when reassigning temp_frontier pile to
//frontier in shortest path operations

void set_in_queue_flags (pile_t** head){

    pile_t* current = *head;
    while(current != NULL){

        current->queued_node->in_queue=true;
        current->queued_node->in_temp_queue = false;
        current=current->next;
    }

}

//used only to reset "in_temp_queue" flags before freeing temp_frontier

void reset_temp_queue_flags (pile_t** head){

    pile_t* current = *head;
    while(current != NULL){
        current->queued_node->in_temp_queue = false;
        current=current->next;
    }

}

//used to free the entire queue at the end of operations


void free_queue(pile_t* head){

    pile_t* tmp;

    while (head != NULL){
        tmp = head;
        head = head->next;
        free(tmp);
    }

}

// car park management

//inserts new car at the given node in decreasing order

int insert_new_car(station_node_t** edited_node , int new_autonomy){

    car_t* newnode = malloc (sizeof(car_t));

    car_t* current = (*edited_node)->car_park;
    car_t* previous = NULL;
    newnode -> autonomy = new_autonomy;


    if (current == NULL) {
        (*edited_node)->car_park = newnode;
        newnode -> next = NULL;
        (*edited_node)->max_reach = (*edited_node)->car_park->autonomy;
        return 0;
    }

    while (current != NULL && current->autonomy > new_autonomy ){
        previous = current;
        current = current->next;
    }



    if (previous == NULL) {
        (*edited_node)->car_park = newnode;
    }
    else previous -> next = newnode;

    newnode -> next = current;
    (*edited_node)->max_reach = (*edited_node)->car_park->autonomy;


    return 0;

}

//deletes car from given node

bool delete_car(station_node_t** edited_node , int autonomy_to_be_deleted){

    car_t* temp = (*edited_node)->car_park;
    car_t* prev = NULL;
    while (temp != NULL){
        if (temp->autonomy == autonomy_to_be_deleted) {
            break;
        }
        if (temp->autonomy < autonomy_to_be_deleted){
            return false;
        }
        prev=temp;
        temp=temp->next;
    }


    if(prev==NULL) {
        if (temp->next != NULL) (*edited_node)->car_park = temp->next;
        else (*edited_node)->car_park = NULL;
    }
    else {
        if (temp->next == NULL) prev->next = NULL;
        else prev->next = temp->next;
    }

    free(temp);

    if ((*edited_node)->car_park == NULL) (*edited_node)->max_reach = 0;
    else (*edited_node)->max_reach = (*edited_node)->car_park->autonomy;
    return true;


}

bool edit_car_park (mode selected_mode, station_node_t** station_node, int new_autonomy){

    if (selected_mode == add){
        if((*station_node)->number_of_cars<512){
            insert_new_car(station_node,new_autonomy);
            (*station_node)->number_of_cars++;
            return true;
        }
        else return false;
    }
    else if (selected_mode == delete){
        if(delete_car(station_node,new_autonomy) == true) {
            (*station_node)->number_of_cars--;
            return true;
        }
        else return false;

    }
    else return false;
}

//used to free an entire car park before station node deletion

bool free_car_park(station_node_t ** head){

    car_t *current = (*head) -> car_park;
    if ( current == NULL) return true;
    car_t *prox = current->next;
    current = (*head)->car_park;
    while(prox != NULL) {
        free(current);

        current=prox;
        prox=prox->next;
    }
    free(current);

    (*head)->car_park = NULL;
    (*head)->number_of_cars = 0;
    return true;
}

//used to delete station node

bool delete_station_node(station_node_t** head, int dist_to_be_deleted){

    station_node_t* node = search_node(*head,dist_to_be_deleted);
    if (node == NULL) return false;
    free_car_park(&node);

    if(node->prev==NULL) {
        if (node->next != NULL) *head = node->next;
        else *head = NULL;
    }
    else{
        if (node->next != NULL) {
            node->prev->next=node->next;
            node->next->prev=node->prev;
        }
        else node->prev->next = NULL;
    }

    free(node);

    return true;
}

//used to free entire station list at the end of the program

void free_station_list(station_node_t** head){

    while (*head != NULL){
        station_node_t * temp = *head;

        free_car_park(head);
        *head = (*head) ->next;

        free(temp);
    }

}

//shortest path algorithm: traces the path with fewer steps between two given nodes in station list

pile_t* shortest_path(station_node_t** start_node, int target){

    direction dir;

    this_operation_id = this_operation_id + 1;

    if ((*start_node)->dist <= target) dir = left_to_right;
    else dir = right_to_left;

    //init path, frontier and visited queues

    pile_t* path=NULL;
    pile_t* frontier=NULL;
    pile_t* visited=NULL;
    pile_t* temp_frontier=NULL;
    station_node_t* target_node=NULL;
    (*start_node)->parent=NULL;

    head_enqueue(&frontier, *start_node);


    if (target==(*start_node)->dist){
        head_enqueue(&path, *start_node);
        return path;
    }

    station_node_t* parent_node;
    station_node_t* current_node;

    while((!isempty(frontier)) || (!isempty(temp_frontier))) {

        while (!isempty(frontier)) {

            parent_node = dequeue(&frontier);
            parent_node->in_queue = false;

            if (dir == left_to_right) current_node = parent_node->next;
            else current_node = parent_node->prev;

            parent_node->visited = true;
            head_enqueue(&visited, parent_node);

            if (current_node != NULL) {

                //scanning adjacent reachable nodes

                while ((dir == left_to_right && (current_node->dist <= (parent_node->dist + parent_node->max_reach))) ||
                       (dir == right_to_left && (current_node->dist >= (parent_node->dist - parent_node->max_reach)))) {

                    //breaks while cycle if current dist is over the target

                    if ((dir == left_to_right && parent_node->dist > target) ||
                        (dir == right_to_left && parent_node->dist < target))
                        break;

                    //updates current node parent if new operation or smaller distance

                    if ((current_node->parent == NULL) || (current_node->last_modified_id != this_operation_id) ||
                        (current_node->last_modified_id == this_operation_id && current_node->in_queue == false && current_node->visited == false && current_node->parent->dist > parent_node->dist)) {
                        current_node->parent = parent_node;
                        current_node->last_modified_id = this_operation_id;
                    }

                    //breaks cycle if target is found

                    if (current_node->dist == target) {
                        target_node = current_node;
                        break;

                    }

                    //if a new node is found adds it to temp_frontier

                    if (current_node->visited == false && current_node->in_temp_queue == false) {

                        if(dir == left_to_right) tail_enqueue(&temp_frontier, current_node);
                        else head_enqueue(&temp_frontier, current_node);
                        current_node->in_temp_queue = true;
                    }

                    if (dir == left_to_right) current_node = current_node->next;
                    else current_node = current_node->prev;
                    if (current_node == NULL) break;

                }


            }


        }

        //when all nodes in frontier are explored, checks if target was found

        if (target_node != NULL){
            reset_temp_queue_flags(&temp_frontier);
            free_queue(temp_frontier);
            break;
        }

        //if target was not found, nodes in temp frontier are moved in frontier

        frontier = temp_frontier;
        set_in_queue_flags(&frontier);
        temp_frontier = NULL;

    }

    if (target_node == NULL) {
        printf("nessun percorso\n");

    }

    else {

        //reconstruct the path
        current_node = target_node;

        while (current_node != NULL) {
            head_enqueue(&path, current_node);
            current_node = current_node->parent;
        }

        print_queue(&path);
        free_queue(path);
    }

    //resets flags and prints the path

    reset_visited_flags(&visited);
    free_queue(visited);
    free_queue(frontier);
    return path;

}





int main() {


    char input[MAX_SIZE];

    int dist, autonomy, start, destination, number_of_cars;
    char *command, *token;

    //init station list
    station_node_t * station_list = NULL;
    station_node_t * new_station;
    station_node_t * start_node;


    while (fgets(input,MAX_SIZE,stdin)!=NULL){

        command = strtok(input, " ");
        if (command == NULL) break;


        if (!strcmp(command, "aggiungi-stazione")) {

            dist = (int)atol(strtok(NULL, " "));
            new_station = create_new_node(dist);
            if(insert_new_station(&station_list,new_station) == true) {

                number_of_cars = (int)atol(strtok(NULL, " "));

                for (int i = 0; i < number_of_cars; i++) {

                    token = strtok(NULL, " ");
                    if(token != NULL ) {
                        autonomy = (int) atol(token);
                        edit_car_park(add, &new_station, autonomy);
                    }

                }
                printf("aggiunta\n");
            }
            else {
                free(new_station);
                printf("non aggiunta\n");
            }
        }

        else if (!strcmp(command, "demolisci-stazione")) {

            dist = (int)atol(strtok(NULL, " "));
            if (delete_station_node(&station_list,dist) == true) printf("demolita\n");
            else printf ("non demolita\n");

        }

        else if (!strcmp(command, "aggiungi-auto")) {

            dist = (int)atol(strtok(NULL, " "));
            new_station = search_node(station_list,dist);
            if (new_station == NULL){
                printf("non aggiunta\n");
            }
            else {
                autonomy = (int)atol(strtok(NULL, " "));
                edit_car_park(add,&new_station,autonomy);
                printf("aggiunta\n");
            }

        }

        else if (!strcmp(command, "rottama-auto")) {

            dist = (int)atol(strtok(NULL, " "));
            new_station = search_node(station_list,dist);
            if (new_station == NULL){
                printf("non rottamata\n");
            }
            else {
                autonomy = (int)atol(strtok(NULL, " "));
                if (edit_car_park(delete,&new_station,autonomy) == true) printf("rottamata\n");
                else printf("non rottamata\n");
            }
        }

        else if (!strcmp(command, "pianifica-percorso")) {

            start = (int)atol(strtok(NULL, " "));
            destination = (int)atol(strtok(NULL, " "));
            start_node = search_node(station_list,start);
            if (start_node == NULL) printf("nessun percorso\n");
            else{
                shortest_path(&start_node,destination);
            }

        }

        else {

            printf("ERROR: unrecognized %s command\n", command);
            //exit(0);

        }

    }
    free_station_list(&station_list);

}


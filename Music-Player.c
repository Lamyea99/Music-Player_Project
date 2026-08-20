#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Song {
  int id;
  char title[50];
} Song;

// 1. Doubly Linked List Node (Playlist)
typedef struct DLLNode {
  Song data;
  struct DLLNode *next;
  struct DLLNode *prev;
} DLLNode;

// 2. Singly Linked List Node (Stack)
typedef struct StackNode {
  Song data;
  struct StackNode *next;
} StackNode;

// 3. Binary Search Tree Node (Library Index)
typedef struct BSTNode {
  Song data;
  struct BSTNode *left;
  struct BSTNode *right;
} BSTNode;

// Player Controller State Structure
typedef struct MusicPlayer {
  BSTNode *library_root;  // BST Search Engine
  DLLNode *playlist_head; // Doubly Linked List Head
  DLLNode *playlist_tail; // Doubly Linked List Tail
  DLLNode *current_track; // Currently Playing Track
  StackNode *history_top; // Stack Top (Recently Played)
  int is_looping;         // Circular List Flag (0 = Off, 1 = On)
} MusicPlayer;


MusicPlayer mp; 
int choice, id;
char title[50];

Song create_song(int id, char *title) {
  Song s;
  s.id = id;
  strcpy(s.title, title);
  return s;
}

void print_song(Song s) {
  printf("[ID: %d] %s\n", s.id, s.title);
}

// --------------------------------------------------
// 1.1 BST INSERT OPERATION
// --------------------------------------------------

BSTNode *insert_bst(BSTNode *root, Song s) {
  if (root == NULL) {
    BSTNode *new_node = malloc(sizeof(BSTNode));
    new_node->data = s;
    new_node->left = new_node->right = NULL;
    return new_node;
  }
  if (s.id < root->data.id) {
    root->left = insert_bst(root->left, s);
  } else if (s.id > root->data.id) {
    root->right = insert_bst(root->right, s);
  }
  return root;
}

// --------------------------------------------------
// 1.2 BST SEARCH OPERATION
// --------------------------------------------------

BSTNode *search_bst(BSTNode *root, int id) {
  if (root == NULL || root->data.id == id) {
    return root;
  }
  if (id < root->data.id) {
    return search_bst(root->left, id);
  }
  return search_bst(root->right, id);
}

// --------------------------------------------------
// 1.3 BST DELETE OPERATION
// --------------------------------------------------

BSTNode *find_min_bst(BSTNode *root) {
  while (root != NULL && root->left != NULL) {
    root = root->left;
  }
  return root;
}

BSTNode *delete_bst(BSTNode *root, int id) {
  if (root == NULL) {
    return NULL;
  }
  if (id < root->data.id) {
    root->left = delete_bst(root->left, id);
  } else if (id > root->data.id) {
    root->right = delete_bst(root->right, id);
  } else {
    // Node found
    if (root->left == NULL) {
      BSTNode *temp = root->right;
      free(root);
      return temp;
    } else if (root->right == NULL) {
      BSTNode *temp = root->left;
      free(root);
      return temp;
    }
    // Node with two children: replace with in-order successor
    BSTNode *succ = find_min_bst(root->right);
    root->data = succ->data;
    root->right = delete_bst(root->right, succ->data.id);
  }
  return root;
}

// --------------------------------------------------
// 1.3 BST TRAVERSAL OPERATION (In-Order)
// --------------------------------------------------

void inorder_bst(BSTNode *root) {
  if (root != NULL) {
    inorder_bst(root->left);
    print_song(root->data);
    inorder_bst(root->right);
  }
}

// --------------------------------------------------
// 2. DOUBLY / CIRCULAR LINKED LIST OPERATIONS (Playlist)
// --------------------------------------------------

void add_to_playlist(Song s) {
  DLLNode *new_node = malloc(sizeof(DLLNode));
  new_node->data = s;
  new_node->next = NULL;
  new_node->prev = NULL;

  if (mp.playlist_head == NULL) {
    mp.playlist_head = new_node;
    mp.playlist_tail = new_node;
    mp.current_track = new_node;
  } else {
    new_node->prev = mp.playlist_tail;
    mp.playlist_tail->next = new_node;
    mp.playlist_tail = new_node;
  }

  // Preserve circular connection if loop mode is enabled
  if (mp.is_looping) {
    mp.playlist_tail->next = mp.playlist_head;
    mp.playlist_head->prev = mp.playlist_tail;
  }
}

int delete_from_playlist(int id) {
  if (mp.playlist_head == NULL) {
    return 0;
  }

  DLLNode *curr = mp.playlist_head;
  DLLNode *target = NULL;

  while (curr != NULL) {
    if (curr->data.id == id) {
      target = curr;
      break;
    }
    curr = curr->next;
    if (curr == mp.playlist_head) {
      break;
    }
  }

  if (target == NULL) {
    return 0;
  }

  // If target is the only node in the playlist
  if (mp.playlist_head == mp.playlist_tail && mp.playlist_head == target) {
    mp.playlist_head = NULL;
    mp.playlist_tail = NULL;
    mp.current_track = NULL;
    free(target);
    return 1;
  }

  // Update current_track if it points to the target node
  if (mp.current_track == target) {
    if (target->next != NULL && target->next != target) {
      mp.current_track = target->next;
    } else if (target->prev != NULL && target->prev != target) {
      mp.current_track = target->prev;
    } else {
      mp.current_track = NULL;
    }
  }

  // If deleting head
  if (target == mp.playlist_head) {
    mp.playlist_head = target->next;
  }

  // If deleting tail
  if (target == mp.playlist_tail) {
    mp.playlist_tail = target->prev;
  }

  // Fix adjacent pointers
  if (target->prev != NULL) {
    target->prev->next = target->next;
  }
  if (target->next != NULL) {
    target->next->prev = target->prev;
  }

  // Maintain circular links if repeat mode is enabled
  if (mp.is_looping && mp.playlist_head != NULL && mp.playlist_tail != NULL) {
    mp.playlist_tail->next = mp.playlist_head;
    mp.playlist_head->prev = mp.playlist_tail;
  } else if (!mp.is_looping) {
    if (mp.playlist_head != NULL) {
      mp.playlist_head->prev = NULL;
    }
    if (mp.playlist_tail != NULL) {
      mp.playlist_tail->next = NULL;
    }
  }

  free(target);
  return 1;
}

void toggle_repeat_mode() {
  if (mp.playlist_head == NULL)
    return;

  mp.is_looping = !mp.is_looping;

  if (mp.is_looping) {
    // Form Circular Linked List
    mp.playlist_tail->next = mp.playlist_head;
    mp.playlist_head->prev = mp.playlist_tail;
    printf("\n--> Repeat Mode ENABLED\n");
  } else {
    // Break Circular Linked List
    mp.playlist_tail->next = NULL;
    mp.playlist_head->prev = NULL;
    printf("\n--> Repeat Mode DISABLED\n");
  }
}

// --------------------------------------------------
// 3. STACK OPERATIONS (Play History)
// --------------------------------------------------

void push_history(Song s) {
  StackNode *new_node = malloc(sizeof(StackNode));
  new_node->data = s;
  new_node->next = mp.history_top;
  mp.history_top = new_node;
}

void display_history() {
  printf("\n--- PLAY HISTORY ---\n");
  if (mp.history_top == NULL) {
    printf("History is empty.\n");
    return;
  }
  StackNode *curr = mp.history_top;
  while (curr != NULL) {
    print_song(curr->data);
    curr = curr->next;
  }
}

// --------------------------------------------------
// CONTROLLER LOGIC
// --------------------------------------------------

void play_next() {
  if (mp.current_track == NULL) {
    printf("\nPlaylist is empty.\n");
    return;
  }

  // Save active track to Play History (Stack)
  push_history(mp.current_track->data);

  // Advance Doubly / Circular List
  if (mp.current_track->next != NULL) {
    mp.current_track = mp.current_track->next;
    printf("\n[Playing Next]: ");
    print_song(mp.current_track->data);
  } else {
    printf("\nEnd of playlist reached.\n");
  }
}

void play_previous() {
  if (mp.current_track == NULL) {
    printf("\nPlaylist is empty.\n");
    return;
  }

  if (mp.current_track->prev != NULL) {
    push_history(mp.current_track->data);
    mp.current_track = mp.current_track->prev;
    printf("\n[Playing Previous]: ");
    print_song(mp.current_track->data);
  } else {
    printf("\nAt the beginning of playlist.\n");
  }
}

void search_and_play_song(int id) {
  // 1. Search in BST Index
  BSTNode *res = search_bst(mp.library_root, id);
  if (res == NULL) {
    printf("\n--> Song ID %d not found in library.\n", id);
    return;
  }

  printf("\n--> Song Found:\n");
  print_song(res->data);

  // 2. Prompt user to play the searched song
  int play_choice;
  printf("\nDo you want to play this song now? (1 = Yes, 0 = No): ");
  scanf("%d", &play_choice);

  if (play_choice != 1) {
    return;
  }

  // 3. Find and play song in Playlist
  DLLNode *curr = mp.playlist_head;
  while (curr != NULL) {
    if (curr->data.id == id) {
      if (mp.current_track != NULL) {
        push_history(mp.current_track->data);
      }
      mp.current_track = curr;
      printf("\n--> Now Playing: ");
      print_song(mp.current_track->data);
      return;
    }
    curr = curr->next;
    if (curr == mp.playlist_head) {
      break;
    }
  }
}

void write_bst_to_file(BSTNode *root, FILE *file) {
  if (root == NULL)
    return;
  write_bst_to_file(root->left, file);
  fprintf(file, "%d,%s\n", root->data.id, root->data.title);
  write_bst_to_file(root->right, file);
}

void save_library_to_file() {
  FILE *file = fopen("songs.txt", "w");
  if (file == NULL) {
    printf("Error: Unable to save library to file.\n");
    return;
  }
  write_bst_to_file(mp.library_root, file);
  fclose(file);
}

void seed_library() {
  Song s1 = create_song(101, "Hotel California");
  Song s2 = create_song(102, "Starboy");
  Song s3 = create_song(103, "Night Changes");
  Song s4 = create_song(104, "Shape of You");

  // Seed BST
  mp.library_root = insert_bst(mp.library_root, s1);
  mp.library_root = insert_bst(mp.library_root, s2);
  mp.library_root = insert_bst(mp.library_root, s3);
  mp.library_root = insert_bst(mp.library_root, s4);

  // Seed Playlist
  add_to_playlist(s1);
  add_to_playlist(s2);
  add_to_playlist(s3);
  add_to_playlist(s4);
}

void load_library_from_file() {
  FILE *file = fopen("songs.txt", "r");
  if (file == NULL) {
    // If file doesn't exist, seed initial data & create file
    seed_library();
    save_library_to_file();
    return;
  }

  int id;
  char title[50];
  while (fscanf(file, "%d,%49[^\r\n]\n", &id, title) == 2) {
    Song s = create_song(id, title);
    mp.library_root = insert_bst(mp.library_root, s);
    add_to_playlist(s);
  }
  fclose(file);
}

void delete_song(int id) {
  BSTNode *found = search_bst(mp.library_root, id);
  if (found == NULL) {
    printf("\n--> Song ID %d not found in library.\n", id);
    return;
  }

  char deleted_title[50];
  strcpy(deleted_title, found->data.title);

  mp.library_root = delete_bst(mp.library_root, id);
  delete_from_playlist(id);
  save_library_to_file();

  printf("\n--> Successfully deleted '%s' (ID: %d) from Library & Playlist!\n", deleted_title, id);
}

// --------------------------------------------------
// MAIN CLI LOOP
// --------------------------------------------------

int main() {
  load_library_from_file();

  while (1) {
    printf("\n==========================================\n");
    printf("         SPOTIFY CLI MUSIC PLAYER         \n");
    printf("==========================================\n");
    if (mp.current_track != NULL) {
      printf(" CURRENT TRACK : ");
      print_song(mp.current_track->data);
    }
    if (mp.is_looping) {
      printf(" REPEAT MODE   : [ON]\n");
    } else {
      printf(" REPEAT MODE   : [OFF]\n");
    }
    printf("------------------------------------------\n");
    printf(" 1. Play Next Track\n");
    printf(" 2. Play Previous Track\n");
    printf(" 3. View Play History\n");
    printf(" 4. Search Library by ID\n");
    printf(" 5. Toggle Repeat Mode\n");
    printf(" 6. Display Entire Library\n");
    printf(" 7. Add New Song to Engine\n");
    printf(" 8. Delete Song by ID\n");
    printf(" 0. Exit\n");
    printf("==========================================\n");
    printf(" Select an option: ");

    if (scanf("%d", &choice) != 1) {
      while (getchar() != '\n')
        ; // Clear input buffer
      continue;
    }

    switch (choice) {
    case 1:
      play_next();
      break;
    case 2:
      play_previous();
      break;
    case 3:
      display_history();
      break;
    case 4: {
      printf("Enter Song ID to search: ");
      scanf("%d", &id);
      search_and_play_song(id);
      break;
    }
    case 5:
      toggle_repeat_mode();
      break;
    case 6:
      printf("\n--- COMPLETE LIBRARY INDEX (BST IN-ORDER) ---\n");
      inorder_bst(mp.library_root);
      break;
    case 7: {
      printf("Enter New Song ID: ");
      scanf("%d", &id);

      if (search_bst(mp.library_root, id) != NULL) {
        printf("\n--> Error: Song ID %d already exists in the library!\n", id);
        break;
      }

      printf("Enter Title: ");
      scanf(" %[^\n]", title);

      Song new_s = create_song(id, title);
      mp.library_root = insert_bst(mp.library_root, new_s);
      add_to_playlist(new_s);
      save_library_to_file();
      printf("\n--> Successfully added '%s' to Library & Playlist!\n", title);
      break;
    }
    case 8: {
      printf("Enter Song ID to delete: ");
      scanf("%d", &id);
      delete_song(id);
      break;
    }
    case 0:
      printf("\nExiting Music Player. Goodbye!\n");
      return 0;
    default:
      printf("\nInvalid option. Try again.\n");
    }
  }

  return 0;
}
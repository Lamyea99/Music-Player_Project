#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------
// DATA STRUCTURE DEFINITIONS
// --------------------------------------------------

typedef struct Song {
  int id;
  char title[50];
  char artist[50];
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

// Player Controller State
typedef struct MusicPlayer {
  BSTNode *library_root;  // BST Search Engine
  DLLNode *playlist_head; // Doubly Linked List
  DLLNode *playlist_tail;
  DLLNode *current_track; // Currently Playing
  StackNode *history_top; // Stack (Recently Played)
  int is_looping;    // Circular List Flag (0 = Off, 1 = On)
} MusicPlayer;

// --------------------------------------------------
// HELPER & UTILITY FUNCTIONS
// --------------------------------------------------

Song create_song(int id, const char *title, const char *artist) {
  Song s;
  s.id = id;
  strncpy(s.title, title, 50);
  strncpy(s.artist, artist, 50);
  return s;
}

void print_song(Song s) {
  printf("[ID: %d] %s - %s\n", s.id, s.title, s.artist);
}

// --------------------------------------------------
// 1.1 BST INSERT OPERATION
// --------------------------------------------------

BSTNode *insert_bst(BSTNode *root, Song s) {
  if (root == NULL) {
    BSTNode *new_node = (BSTNode *)malloc(sizeof(BSTNode));
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

void append_playlist(MusicPlayer *mp, Song s) {
  DLLNode *new_node = (DLLNode *)malloc(sizeof(DLLNode));
  new_node->data = s;
  new_node->next = NULL;
  new_node->prev = NULL;

  if (mp->playlist_head == NULL) {
    mp->playlist_head = new_node;
    mp->playlist_tail = new_node;
    mp->current_track = new_node;
  } else {
    new_node->prev = mp->playlist_tail;
    mp->playlist_tail->next = new_node;
    mp->playlist_tail = new_node;
  }

  // Preserve circular connection if loop mode is enabled
  if (mp->is_looping) {
    mp->playlist_tail->next = mp->playlist_head;
    mp->playlist_head->prev = mp->playlist_tail;
  }
}

void toggle_repeat_mode(MusicPlayer *mp) {
  if (mp->playlist_head == NULL)
    return;

  mp->is_looping = !mp->is_looping;

  if (mp->is_looping) {
    // Form Circular Linked List
    mp->playlist_tail->next = mp->playlist_head;
    mp->playlist_head->prev = mp->playlist_tail;
    printf("\n--> Repeat Mode ENABLED (Circular Loop Active)\n");
  } else {
    // Break Circular Linked List
    mp->playlist_tail->next = NULL;
    mp->playlist_head->prev = NULL;
    printf("\n--> Repeat Mode DISABLED (Standard List Active)\n");
  }
}

// --------------------------------------------------
// 3. STACK OPERATIONS (Play History)
// --------------------------------------------------

void push_history(MusicPlayer *mp, Song s) {
  StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));
  new_node->data = s;
  new_node->next = mp->history_top;
  mp->history_top = new_node;
}

void display_history(MusicPlayer *mp) {
  printf("\n--- PLAY HISTORY (STACK) ---\n");
  if (mp->history_top == NULL) {
    printf("History is empty.\n");
    return;
  }
  StackNode *curr = mp->history_top;
  while (curr != NULL) {
    print_song(curr->data);
    curr = curr->next;
  }
}


// --------------------------------------------------
// CONTROLLER LOGIC
// --------------------------------------------------

void play_next(MusicPlayer *mp) {
  if (mp->current_track == NULL) {
    printf("\nPlaylist is empty.\n");
    return;
  }

  // Save active track to Play History (Stack)
  push_history(mp, mp->current_track->data);

  // Advance Doubly / Circular List
  if (mp->current_track->next != NULL) {
    mp->current_track = mp->current_track->next;
    printf("\n[Playing Next]: ");
    print_song(mp->current_track->data);
  } else {
    printf("\nEnd of playlist reached.\n");
  }
}

void play_previous(MusicPlayer *mp) {
  if (mp->current_track == NULL) {
    printf("\nPlaylist is empty.\n");
    return;
  }

  if (mp->current_track->prev != NULL) {
    push_history(mp, mp->current_track->data);
    mp->current_track = mp->current_track->prev;
    printf("\n[Playing Previous]: ");
    print_song(mp->current_track->data);
  } else {
    printf("\nAt the beginning of playlist.\n");
  }
}

void search_and_play_song(MusicPlayer *mp, int id) {
  // 1. Search in BST Index
  BSTNode *res = search_bst(mp->library_root, id);
  if (res == NULL) {
    printf("\n--> Song ID %d not found in library.\n", id);
    return;
  }

  printf("\n--> Song Found in BST Index:\n");
  print_song(res->data);

  // 2. Prompt user to play the searched song
  printf("\nDo you want to play this song now? (1 = Yes, 0 = No): ");
  int play_choice;
  if (scanf("%d", &play_choice) == 1 && play_choice == 1) {
    DLLNode *curr = mp->playlist_head;
    while (curr != NULL) {
      if (curr->data.id == id) {
        if (mp->current_track != NULL) {
          push_history(mp, mp->current_track->data);
        }
        mp->current_track = curr;
        printf("\n--> Now Playing: ");
        print_song(mp->current_track->data);
        return;
      }
      curr = curr->next;
      if (curr == mp->playlist_head) {
        break; // Stop if loop mode is active and wrapped around
      }
    }
  }
}

void seed_library(MusicPlayer *mp) {
  Song s1 = create_song(101, "Hotel California", "Eagles");
  Song s2 = create_song(102, "Starboy", "The Weeknd");
  Song s3 = create_song(103, "Night Changes", "One Direction");
  Song s4 = create_song(104, "Shape of You", "Ed Sheeran");

  // Seed BST
  mp->library_root = insert_bst(mp->library_root, s1);
  mp->library_root = insert_bst(mp->library_root, s2);
  mp->library_root = insert_bst(mp->library_root, s3);
  mp->library_root = insert_bst(mp->library_root, s4);

  // Seed Playlist
  append_playlist(mp, s1);
  append_playlist(mp, s2);
  append_playlist(mp, s3);
  append_playlist(mp, s4);
}

// --------------------------------------------------
// MAIN CLI LOOP
// --------------------------------------------------

int main() {
  MusicPlayer mp = {NULL, NULL, NULL, NULL, NULL, 0};
  seed_library(&mp);

  int choice, id;
  char title[50], artist[50];

  while (1) {
    printf("\n==========================================\n");
    printf("         SPOTIFY CLI MUSIC PLAYER         \n");
    printf("==========================================\n");
    if (mp.current_track != NULL) {
      printf(" CURRENT TRACK : ");
      print_song(mp.current_track->data);
    }
    printf(" REPEAT MODE   : [%s]\n",
           mp.is_looping ? "ON (Circular)" : "OFF (Standard)");
    printf("------------------------------------------\n");
    printf(" 1. Play Next Track          (DLL / Circular)\n");
    printf(" 2. Play Previous Track      (DLL / Circular)\n");
    printf(" 3. View Play History        (Stack Display)\n");
    printf(" 4. Search Library by ID     (BST Search)\n");
    printf(" 5. Toggle Repeat Mode       (Circular List Toggle)\n");
    printf(" 6. Display Entire Library   (BST In-Order)\n");
    printf(" 7. Add New Song to Engine   (BST + DLL)\n");
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
      play_next(&mp);
      break;
    case 2:
      play_previous(&mp);
      break;
    case 3:
      display_history(&mp);
      break;
    case 4: {
      printf("Enter Song ID to search: ");
      scanf("%d", &id);
      search_and_play_song(&mp, id);
      break;
    }
    case 5:
      toggle_repeat_mode(&mp);
      break;
    case 6:
      printf("\n--- COMPLETE LIBRARY INDEX (BST IN-ORDER) ---\n");
      inorder_bst(mp.library_root);
      break;
    case 7: {
      printf("Enter New Song ID: ");
      scanf("%d", &id);
      printf("Enter Title: ");
      scanf("%s", title);
      printf("Enter Artist: ");
      scanf("%s", artist);

      Song new_s = create_song(id, title, artist);
      mp.library_root = insert_bst(mp.library_root, new_s);
      append_playlist(&mp, new_s);
      printf("\n--> Successfully added '%s' to Library & Playlist!\n", title);
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
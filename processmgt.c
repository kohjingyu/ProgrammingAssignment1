/* Programming Assignment 1
∗ Author: Koh Jing Yu
∗ ID: 1002045
∗ Date: 02/02/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

#define MAX_LENGTH 1024
#define MAX_PARENTS 10
#define MAX_CHILDREN 10
#define MAX_NODES 50

typedef struct node {
  int id;
  char prog[MAX_LENGTH];
  char *args[MAX_LENGTH/2 + 1];
  int num_args;
  char input[MAX_LENGTH];
  char output[MAX_LENGTH];
  int parents[MAX_PARENTS];
  int num_parents;
  int children[MAX_CHILDREN];
  int num_children;
  int status;
  pid_t pid;
} node_t;

/**
 * Search for tokens in the string s, separated by the characters in 
 * delimiters. Populate the string array at *tokens.
 *
 * Return the number of tokens parsed on success, or -1 and set errno on 
 * failure.
 */
int parse_tokens(const char *s, const char *delimiters, char ***tokens) {
  const char *s_new;
  char *t;
  int num_tokens;
  int errno_copy;

  /* Check arguments */
  if ((s == NULL) || (delimiters == NULL) || (tokens == NULL)) {
    errno = EINVAL;
    return -1;
  }

  /* Clear token array */
  *tokens = NULL;

  /* Ignore initial segment of s that only consists of delimiters */
  s_new = s + strspn(s, delimiters);

  /* Make a copy of s_new (strtok modifies string) */
  t = (char *) malloc(strlen(s_new) + 1);
  if (t == NULL) {
    return -1;    
  }
  strcpy(t, s_new);

  /* Count number of tokens */
  num_tokens = 0;
  if (strtok(t, delimiters) != NULL) {
    for (num_tokens = 1; strtok(NULL, delimiters) != NULL; num_tokens++) ;
  }

  /* Allocate memory for tokens */
  *tokens = (char**) malloc((num_tokens + 1)*sizeof(char *));
  if (*tokens == NULL) {
    errno_copy = errno;
    free(t);  // ignore errno from free
    errno = errno_copy;  // retain errno from malloc
    return -1;
  }

  /* Parse tokens */
  if (num_tokens == 0) {
    free(t);
  } else {
    strcpy(t, s_new);
    **tokens = strtok(t, delimiters);
    for (int i=1; i<num_tokens; i++) {
      *((*tokens) +i) = strtok(NULL, delimiters);      
    }
  }
  *((*tokens) + num_tokens) = NULL;  // end with null pointer

  return num_tokens;
}

void free_parse_tokens(char **tokens) {
  if (tokens == NULL) {
    return;    
  }
  
  if (*tokens != NULL) {
    free(*tokens);    
  }

  free(tokens);
}

/**
 * Parse the input line at line, and populate the node at node, which will
 * have id set to id.
 * 
 * Return 0 on success or -1 and set errno on failure.
 */
int parse_input_line(char *line, int id, node_t *node) {
  char **strings;  // string array
  char **arg_list;  // string array
  char **child_list;  // string array
  int a;

  /* Split the line on ":" delimiters */
  if (parse_tokens(line, ":", &strings) == -1) {
    perror("Failed to parse node information");
    return -1;
  }

  /* Parse the space-delimited argument list */
  if (parse_tokens(strings[0], " ", &arg_list) == -1) {
    perror("Failed to parse argument list");
    free_parse_tokens(strings);
    return -1;
  }

  /* Parse the space-delimited child list */
  if (parse_tokens(strings[1], " ", &child_list) == -1) {
    perror("Failed to parse child list");
    free_parse_tokens(strings);
    return -1;
  }

  /* Set node id */
  node->id = id;
  fprintf(stderr, "... id = %d\n", node->id);

  /* Set program name */
  strcpy(node->prog, arg_list[0]);
  fprintf(stderr, "... prog = %s\n", node->prog);

  /* Set program arguments */
  for (a = 0; arg_list[a] != NULL; a++) {
    node->args[a] = arg_list[a];
    node->num_args++;
    fprintf(stderr, "... arg[%d] = %s\n", a, node->args[a]);
  }
  node->args[a] = NULL;
  fprintf(stderr, "... arg[%d] = %s\n", a, node->args[a]);

  /* Set input file */
  strcpy(node->input, strings[2]);
  fprintf(stderr, "... input = %s\n", node->input);
  
  /* Set output file */
  strcpy(node->output, strings[3]);
  fprintf(stderr, "... output = %s\n", node->output);

  fprintf(stderr, "... status = %d\n", node->status);
    
  /* Set child nodes */
  node->num_children = 0;
  if (strcmp(child_list[0], "none") != 0) {
    for (int c = 0; child_list[c] != NULL; c++) {
      if (c < MAX_CHILDREN) {
        if (atoi(child_list[c]) != id) {
          node->children[c] = atoi(child_list[c]);
          fprintf(stderr, "... child[%d] = %d\n", c, node->children[c]);
          node->num_children++;
        } else {
          perror("Node cannot be a child of itself");
          return -1;
        }
      } else {
        perror("Exceeded maximum number of children per node");
        return -1;
      }
    }
  }
  fprintf(stderr, "... num_children = %d\n", node->num_children);

  return 0;
}

/**
 * Parse the file at file_name, and populate the array at n.
 * 
 * Return the number of nodes parsed on success, or -1 and set errno on
 * failure.
 */
int parse_graph_file(char *file_name, node_t *node) {
  FILE *f;
  char line[MAX_LENGTH];
  int id = 0;
  int errno_copy;

  /* Open file for reading */
  fprintf(stderr, "Opening file...\n");
  f = fopen(file_name, "r");
  if (f == NULL) {
    perror("Failed to open file");
    return -1;
  }

  /* Read file line by line */
  fprintf(stderr, "Reading file...\n");
  while (fgets(line, MAX_LENGTH, f) != NULL) {
    strtok(line, "\n");  // remove trailing newline

    /* Parse line */
    fprintf(stderr, "Parsing line %d...\n", id);
    if (parse_input_line(line, id, node) == 0) {
      node++;  // increment pointer to point to next node in array
      id++;  // increment node ID
      if (id >= MAX_NODES) {
        perror("Exceeded maximum number of nodes");
        return -1;
      }
    } else {
      perror("Failed to parse input line");
      return -1;
    }
  }

  /* Handle file reading errors and close file */
  if (ferror(f)) {
    errno_copy = errno;
    fclose(f);  // ignore errno from fclose
    errno = errno_copy;  // retain errno from fgets
    perror("Error reading file");
    return -1;
  }

  /* If no file reading errors, close file */
  if (fclose(f) == EOF) {
    perror("Error closing file");
    return -1;  // stream was not successfully closed
  }
  
  /* If no file closing errors, return number of nodes parsed */  
  return id;
}

/**
 * Parses the process tree represented by nodes and determines the parent(s)
 * of each node.
 */
int parse_node_parents(node_t *nodes, int num_nodes) {
  for(int i = 0; i < num_nodes; i ++) {
    node_t *parent_node = &nodes[i]; // Retrieve a pointer to this node

    fprintf(stderr, "Parsing node %d\n", parent_node->id);
    // For each children this node has, set its parent to parent_node
    for(int c = 0; c < parent_node->num_children; c ++) {
      // Since nodes is in order of id, we can get the child using its id
      int child_id = parent_node->children[c];

      // Child id is invalid
      if(child_id >= num_nodes) {
        perror("Failed to parse node parents");
        return -1;
      }

      node_t *child_node = &nodes[child_id];

      // Set parent of this child to parent_node
      int parent_index = child_node->num_parents; // Get the index of the new parent
      child_node->parents[parent_index] = parent_node->id; // Set the parent of the child
      child_node->num_parents += 1; // Increment number of parents of this child_node

      fprintf(stderr, "... parents[%d] of node %d = %d\n", parent_index, child_node->id, i);
    }
  }

  return 0;
}

/**
 * Checks the status of each node in the process tree represented by nodes and 
 * verifies whether it can progress to the next stage in the cycle:
 *
 * INELIGIBLE -> READY -> RUNNING -> FINISHED
 *
 * Returns the number of nodes that have finished running, or -1 if there was 
 * an error.
 */
int parse_node_status(node_t *nodes, int num_nodes) {
  int num_finished = 0;
  for(int i = 0; i < num_nodes; i ++) {
    node_t *node = &nodes[i]; // Retrieve a pointer to this node

    // Check if node is finished running
    if(node->status == FINISHED) {
      num_finished += 1;
    }
    else if(node->status == RUNNING) {
      // TODO: check if it is done running
      fprintf(stderr, "Running %d\n", node->id);

      node->pid = fork();
      if(node->pid == -1) {
        // Some error occured, failed to fork
        return -1;
      }
      else if(node->pid > 0) {
        // This is the parent, wait for child to finish
        int status;
        pid_t return_pid = waitpid(node->pid, &status, 0);

        // Error
        if(WEXITSTATUS(status) != 0 || WIFEXITED(status) == 0) {
          // waitpid failed
          return -1;
        }

        node->status = FINISHED; // Successfully finished child process
      }
      else {
        // This is the child. Run the program.
        // Manage input / output
        if(strcmp(node->input, "stdin") != 0) {
          // STDIN
          // Not stdin -> redirect input from stdin to file
          fprintf(stderr, "Redirecting stdin to %s\n", node->input);

          int file = open(node->input, O_RDONLY);
          dup2(file, 0); // Get stdin from file
          close(file);
        }

        // Redirect output to file
        if(strcmp(node->output, "stdout") != 0) {
          // STDOUT
          // Open output file
          fprintf(stderr, "Redirecting %s to stdout\n", node->output);
          int file = open(node->output, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
          dup2(file, 1); // Make stdout output to file
          dup2(file, 2); // Make stderr output to file
          close(file);
        }

        execvp(node->prog, node->args);
        exit(errno); // Failed to exec command
      }
    }
    else if(node->status == READY) {
      // Ready to run
      node->status = RUNNING;
    }
    else { // node-> status == INELIGIBLE
      // Check if this node's status can be changed to READY
      // No parent, elligible to run
      if(node->num_parents == 0) {
        node->status = READY;
      }
      else {
        int parents_done = 1; // Set to true by default

        // Check if all parents are done running
        for(int p = 0; p < node->num_parents; p ++) {
          int parent_id = nodes->parents[p]; // Get the parent id
          node_t *parent_node = &nodes[parent_id]; // Retrieve a pointer to the parent node
          // If we find a parent that has not completed running, set parents_done to false
          if(parent_node->status != FINISHED) {
            parents_done = 0;
            // We can stop checking at this point as the node will be ineligble, so we break the loop
            break;
          }
        }

        // All parents are done: set node to READY
        if(parents_done == 1) {
          node->status = READY;
        }
      }
    }
  }

  return num_finished;
}

/**
 * Prints the process tree represented by nodes to standard error.
 *
 * Returns 0 if printed successfully.
 */
int print_process_tree(node_t *nodes, int num_nodes) {
  for(int i = 0; i < num_nodes; i ++) {
    node_t *node = &nodes[i]; // Retrieve a pointer to this node

    // Print ID of this node
    fprintf(stderr, "Node: %d \n", node->id);

    /*** PRINT PARENTS OF NODE ***/
    fprintf(stderr, "Parents: ");
    // No parents, print none
    if(node->num_parents == 0) {
      fprintf(stderr, "none");
    }

    for(int p = 0; p < node->num_parents; p ++) {
      node_t *parent_node = &nodes[p]; // Retrieve a pointer to the parent node
      fprintf(stderr, "%d", parent_node->id);

      if(p < node->num_parents - 1) {
        fprintf(stderr, ", "); // Print a comma if it's not the last parent
      }
    }

    fprintf(stderr, "\n"); // Print an endline after the last parent is output
    /*** END PRINT PARENTS ***/

    /*** PRINT CHILDREN OF NODE ***/
    fprintf(stderr, "Children: ");
    // No parents, print none
    if(node->num_children == 0) {
      fprintf(stderr, "none");
    }

    for(int c = 0; c < node->num_children; c ++) {
      node_t *child_node = &nodes[c]; // Retrieve a pointer to the child node
      fprintf(stderr, "%d", child_node->id);

      if(c < node->num_children - 1) {
        fprintf(stderr, ", "); // Print a comma if it's not the last child
      }
    }

    fprintf(stderr, "\n"); // Print an endline after the last child is output
    /*** END PRINT CHILDREN ***/

    /*** PRINT COMMAND ***/
    fprintf(stderr, "Command: ");

    // Output all arguments
    for(int a = 0; a < node->num_args; a ++) {
      fprintf(stderr, "%s ", node->args[a]);
    }

    fprintf(stderr, "\n");
    /*** END PRINT COMMAND ***/

    // Print input file
    fprintf(stderr, "Input file: %s \n", node->input);
    // Print output file
    fprintf(stderr, "Output file: %s \n", node->output);

    /*** PRINT RUNNABLE ***/
    fprintf(stderr, "Runnable: ");

    if(node->status == READY) {
      fprintf(stderr, "Yes\n");
    }
    else {
      fprintf(stderr, "No\n");
    }
    /*** END PRINT RUNNABLE ***/

    /*** PRINT EXECUTED ***/
    fprintf(stderr, "Executed: ");

    if(node->status == FINISHED) {
      fprintf(stderr, "Yes\n");
    }
    else {
      fprintf(stderr, "No\n");
    }
    /*** END PRINT RUNNABLE ***/

    // After each node, print a line to format it nicely
    fprintf(stderr, "--------------------\n");
  }

  return 0;
}

/**
 * Takes in a graph file and executes the programs in parallel.
 */
int main(int argc, char *argv[]) {
  node_t nodes[MAX_NODES];
  int num_nodes;
  int num_nodes_finished;

  /* Check command line arguments */
  char *file_name = argv[1];

  if(file_name == NULL) {
    perror("Error: please supply a graph file.");
    return EXIT_FAILURE;
  }

  /* INSERT CODE */

  /* Parse graph file */
  fprintf(stderr, "Parsing graph file...\n");
  num_nodes = parse_graph_file(file_name, nodes);
  fprintf(stderr, "====================\n");

  /* Parse nodes for parents */
  fprintf(stderr, "Parsing node parents...\n");
  parse_node_parents(nodes, num_nodes);
  fprintf(stderr, "====================\n");

  /* Print process tree */
  fprintf(stderr, "Process tree:\n");
  /* print the process tree */
  print_process_tree(nodes, num_nodes);
  fprintf(stderr, "====================\n");

  /* Run processes */
  fprintf(stderr, "Running processes...\n");

  // Run until there is an error or all processes have finished
  while(num_nodes_finished < num_nodes && num_nodes_finished >= 0) {
    /* invocation to parse_node_status */
    num_nodes_finished = parse_node_status(nodes, num_nodes);

    if(num_nodes_finished == -1) {
    	// Error occurred
    	break;
    }
  }

  if (num_nodes_finished < 0) {
    perror("Error executing processes");
    return EXIT_FAILURE;
  }

  fprintf(stderr, "All processes finished. Exiting.\n");
  return EXIT_SUCCESS;
}

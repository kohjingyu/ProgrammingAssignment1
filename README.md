# 50.005 Programming Assignment 1

**Author:** Koh Jing Yu (1002045)

The purpose of this programming assignment is to develop a program that uses UNIX system calls to traverse a directed acyclic graph of user programs. This graph file is read as a text file, and the commands are executed in an order such that all dependencies are satisfied.

The `processmgt` program is written in C.

## Prerequisites

`gcc` is required to build the program.

## Compiling the Program

The program is compiled with `gcc`:

```sh
gcc processmgt.c -o processmgt.bin
```

## Running the Program

After compilation, the program can be run with a specified graph file. For example, to run the program on `graph-file.dms`, run the following command in Terminal:

```sh
./processmgt.bin graph-file.dms
```

## Program Functionality

### Parsing of Graph Files

`processmgt` takes in a graph file as input. It parses this graph file to determine the process nodes that are present within the file, and the dependencies that each node has.

For example, a definition of a node within a graph file may be as follows:

```
sleep 15:1:stdin:stdout
```

This means that the command to execute is `sleep 15`, this node is the parent of **node 1**, and takes input from **stdin** and outputs to **stdout**. The **id** of this node is determined by its position within the graph file. For example, if this node is the first line, it will be **node 0**.

For each node, its data is stored within a `struct` with the following fields:

|Field name   |Type       |Description                                                      |
|-------------|-----------|-----------------------------------------------------------------|
|id           |int        |Unique identifier for each node.                                 |
|prog         |char array |Stores the program to be run by this node.                       |
|args         |char array |Stores the arguments to be passed into prog.                     |
|num_args     |int        |The number of arguments.                                         |
|input        |char array |Input file to the program (or stdin)                             |
|output       |char array |Output file of the program (or stdout)                           |
|parents      |int array  |Stores all the parent ids of this node.                          |
|num_parents  |int        |Number of parents of this node.                                  |
|children     |int array  |Stores all the children ids of this node.                        |
|num_children |int        |Number of children of this node.                                 |
|status       |int        |The status of this node (INELIGIBLE, READY, RUNNING, FINISHED)   |
|pid          |pid_t      |Stores the process id of this node (for forking purposes).       |


```c
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
```

### Parsing of Node Parents

After all nodes have been parsed, they will be iterated through again to determine the parent of each node. Since we already accounted for each node's children while parsing the graph file, this is straightforward.

For each node, the program loops through each of its children, and adds the current node as a parent for that child.

### Running Nodes

After parsing is completed, the nodes are then processed in order. This involves a `while` loop in the main body of the code that iterates until all nodes have finished running, or if there is an error in running one node:

```c
// Run until there is an error or all processes have finished
while(num_nodes_finished < num_nodes && num_nodes_finished >= 0) {
  /* invocation to parse_node_status */
  num_nodes_finished = parse_node_status(nodes, num_nodes);

  if(num_nodes_finished == -1) {
  	// Error occurred
  	break;
  }
```

The `parse_node_status` function loops through all nodes to determine the nodes that are eligible to run. The instructions applied to each node depend on its status:

* If the status is `FINISHED`, it increments the number of finished nodes and moves on to the next node, as this node does not need to be further processed.
* `RUNNING`: the node is undergoing execution. The program uses the `fork` command to run the process. It uses `dup2` to redirect input and output of the process. If the `fork` exits with an error, `processmgt.c` is ended and the error is reported to the user.
* `READY` This node is ready to run. Its status is simply changed to `RUNNING` such that it will be run in the next cycle.
* `INELIGIBLE` This node was previously ineligible to run in the previous cycle. Iterate through each of the node's parents, and determined if their status is `FINISHED`. If all parents of this node are `FINISHED`, it is eligible to run. Its status is set to `READY`, and it will begin execution in the next cycle.

After all nodes have been examined, the function returns the number of nodes that have completed execution.

## Conclusion

The program parses a graph file and stores the information within an array. Execution is performed by determining which nodes are eligible to run (i.e. no parents, or all parents have finished). By doing so, it ensures that the execution order is in a sequence such that all dependencies will be satisfied.

## Acknowledgements

* My 50.005 professors for teaching me the knowledge I needed for completing this project.
* Our TA, Benjamin Kang, for helping with the many questions I had about the project.

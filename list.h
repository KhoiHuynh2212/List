typedef enum node_kind_t {
    INTEGER = 0, 
    DOUBLE = 1,
    FLOAT = 2, 
    STRING = 3
} node_kind_t;

typedef union node_data {
    int node_int;
    double node_double;
    float node_float;
    char* node_string;
} node_data;

typedef struct Node {
    struct Node * next;
    node_kind_t kind;
    node_data data;

} Node; 

Node* create_int_node(int data);
Node* create_double_node(double data);
Node* create_string_node(char* data);
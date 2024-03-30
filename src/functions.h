typedef struct function_metadata  function_metadata_t;
typedef struct function_def       function_def_t;

struct function_def
{
  const char *name;
  segoff_t addr;
};


struct function_metadata
{
  size_t           n_defs;
  function_def_t * defs;
};

void hydra_function_metadata_init(void);

const function_def_t * function_find(const char *name);
const char *           function_name(segoff_t s);
bool                   function_addr(const char *name, segoff_t *_out);

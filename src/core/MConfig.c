#include "MConfig.h"

/**
 * @fn void ConfigMessage(int j, int type, char* message)
 * @brief Prints error messages.
 * 
 * @param j
 *  The line in the config file where the error exist.
 *
 * @param type
 *  Type of the error.
 *
 * @param message
 *  Error message to print.
 */
void ConfigMessage(int line, int type, char* message){

  switch(type){
    case LRC_ERR_CONFIG_SYNTAX:
      printf("%s at line %d: %s\n", LRC_MSG_CONFIG_SYNTAX, line, message);
      break;
    case LRC_ERR_FILE_OPEN:
      printf("%s at line %d: %s\n", LRC_MSG_FILE_OPEN, line, message);
      break;
    case LRC_ERR_HDF:
      printf("%s at line %d: %s\n", LRC_MSG_HDF, line, message);
      break;
    default:
      break;
  }

}

/**
 * @fn char* ConfigTrim(char* str)
 *
 * @brief Remove leading, trailing, & excess embedded spaces.
 *
 * Public domain by Bob Stout.
 * @see http://www.cs.umu.se/~isak/snippets/trim.c
 *
 * @param str
 *   String to trim.
 */
char* ConfigTrim(char* str){

  char *ibuf = str, *obuf = str;
  int i = 0, cnt = 0;

  /* Trap NULL.*/
  if (str != NULL){
 
    /* Remove leading spaces (from RMLEAD.C).*/
    for (ibuf = str; *ibuf && isspace(*ibuf); ++ibuf)
      ;
    if (str != ibuf)
      memmove(str, ibuf, ibuf - str);

    /* Collapse embedded spaces (from LV1WS.C).*/
    while (*ibuf){
      if (isspace(*ibuf) && cnt) ibuf++;
        else{
          if (!isspace(*ibuf)) cnt = 0;
            else{
              *ibuf = ' ';
              cnt = 1;
              }
            obuf[i++] = *ibuf++;
            }
          }
          obuf[i] = CONFIG_NULL;

     /* Remove trailing spaces (from RMTRAIL.C).*/
     while (--i >= 0) { if (!isspace(obuf[i])) break;}
     obuf[++i] = CONFIG_NULL;
    }

  return str;
}

/**
 * @fn char* ConfigNameTrim(char* l)
 * @brief Check namespace name and trim it.
 *
 * @param l
 *   Name of the namespace.
 *
 * @return
 *   Trimmed name of the namespace.
 */
char* ConfigNameTrim(char* l){

  int len = 0;

  len = strlen(l);

  /* Quick and dirty solution using trim function. */
  l[0] = ' ';
  l[len-1] = ' ';
  l = ConfigTrim(l);  

  return l;
}

/**
 * @fn int ConfigCharCount(char* l, char* s)
 * @brief Count the number of separators in the current line.
 *
 * @param l
 *   Current line.
 *
 * @param s
 *   The separator to count.
 *
 * @return
 *   Number of separators in the current line.
 */
int ConfigCharCount(char* l, char* s){
  
  int i = 0; int sep = 0; int len = 0; 

  len = strlen(l);

  for (i = 0; i < len; i++){
    if((int) *s == l[i] ) sep++;
  }
  
  return sep;
}

/**
 * @fn int LRC_checkName(char* varname, options* ct, int numCT)
 * @brief Checks if variable is allowed.
 *
 * @param varname
 *   The name of the variable to check.
 *
 * @param ct
 *   Pointer to the structure with allowed config datatypes.
 *
 * @param numCT
 *   Number of allowed config datatypes.
 *
 * @return
 *   0 on success, -1 otherwise.  
 */
/*int LRC_checkName(char* varname, options* ct, int numCT){
   
  int i = 0, count = 0;

  for(i = 0; i < numCT; i++){
      if(strcmp(varname, ct[i].name) == 0) count++;
  }
  
  if(count > 0) 
    return 0;
  else
    return -1;

}
*/
/**
 * @fn int LRC_matchType(char* varname, char* value, options* ct, int numCT)
 * @brief Match input type.
 *
 * @param varname
 *   Variable name.
 *
 * @param value
 *   The value of the variable.
 *
 * @param ct
 *   Pointer to the structure with allowed config datatypes.
 *
 * @param numCT
 *   Number of config datatypes allowed.
 *   
 * @return
 *   Returns type (integer value) on success, -1 otherwise.
 *   @see options
 */
/*int LRC_matchType(char* varname, char* value, options* ct, int numCT){
  
  int i = 0;

  while(i < numCT){
    if(strcmp(ct[i].name,varname) == 0){
      if(LRC_checkType(value, ct[i].type) != 0){ 
        return -1;
      }else{
        return ct[i].type;
      }
    }
      i++;
  }
  return 0;

}
*/
/**
 * @fn int LRC_checkType(char* value, int type)
 * @brief Check type of the value.
 *
 * @param value
 *   The value to check.
 *
 * @param type
 *   The datatype for given value.
 *   @see options
 *
 * @return
 *   0 on success, -1 otherwise.
 */
/*int LRC_checkType(char* value, int type){
  
  int i = 0, ret = 0, k = 0;
  char *p;

  switch(type){
    
    case C_INT:
      for(i = 0; i < strlen(value); i++){
        if(isdigit(value[i]) || value[0] == '-'){ 
          ret = 0;
        }else{
          ret = -1;
          break;
        }
      }
      break;

    case LRC_FLOAT:
      k = strtol(value, &p, 10);
      if(value[0] != '\n' && (*p == '\n' || *p != '\0')){
        ret = 0;
      }else{
        ret = -1;
      }
      break;

    case T_DOUBLE:
      k = strtol(value, &p, 10);
      if(value[0] != '\n' && (*p == '\n' || *p != '\0')){
        ret = 0;
      }else{
        ret = -1;
      }
      break;

    case T_STRING:
      for(i = 0; i < strlen(value); i++){
        if(isalpha(value[i]) || LRC_isAllowed(value[i]) == 0){
          ret = 0;
        }else{
          ret = -1;
          break;
        }
      }
      break;
  
    default:
      break;

  }

  return ret;

}
*/
/** 
 * @fn int LRC_isAllowed(int c)
 * @brief Check if given char is one of the allowed chars.
 *
 * @param c
 *   The char to check.
 *
 * @return
 *   0 on succes, -1 otherwise.
 *
 * @todo
 *   Allow user to override allowed char string.
 */
/*int LRC_isAllowed(int c){

  char* allowed = "_-. ";
  int i = 0;

  for(i = 0; i < strlen(allowed); i++){
    if(c == allowed[i]) return 0;
 }

  return 1;
}
*/
/**
 * @fn void newNamespace(char* cfg)
 * @brief Helper function for creating new namespaces
 */
configNamespace* ConfigNewNamespace(char* cfg) {

  configNamespace* newNM = NULL;

  newNM = calloc(sizeof(configNamespace), sizeof(configNamespace));
  if (!newNM) {
    perror("ConfigNewNamespace: line 374 alloc failed.");
    return NULL;
  }

  strncpy(newNM->space, cfg, strlen(cfg));
  newNM->space[strlen(cfg)] = CONFIG_NULL;
  newNM->options = NULL;
  newNM->next = NULL;

  return newNM;
}

/**
 * @}
 */

/**
 * @defgroup LRC_userAPI User API
 * @{
 * Public User API.
 */

/**
 *  @defgroup LRC_parser Parsers
 *  @{
 *  Currently there are two parsers available:
 *  - Text file parser @see LRC_ASCIIParser()
 *  - HDF5 file parser @see LRC_HDF5Parser()
 *
 *  @todo
 *    Add secondary separator support.
 */

/**
 *  Text parser
 *
 *  @fn int LRC_ASCIIParser(FILE* read, char* SEP, char* COMM, configNamespace* head)
 *  @brief Reads config file, namespaces, variable names and values,
 *  into the options structure @see configNamespace.
 *
 *  @param read
 *    Handler of the config file to read.
 *    
 *  @param SEP
 *    The separator name/value.
 *
 *  @param COMM
 *    The comment mark.
 *
 *  @param head
 *    Pointer to the structure with datatypes allowed in the config file.
 *
 *  @return
 *    Number of namespaces found in the config file on success, -1 otherwise.
 *
 */

int ConfigAsciiParser(FILE* read, char* SEP, char* COMM, configNamespace* head){
  
  int j = 0; int sepc = 0; int n = 0;
  char* line; char l[CONFIG_MAX_LINE_LENGTH]; char* b; char* c;
  char* value; 

  config* newOP = NULL;
  configNamespace* nextNM = NULL;
  configNamespace* current = NULL;

  size_t valuelen = 0;

  if (!head) {
    perror("LRC_ASCIIParser: No config assigned");
    return -1;
  }

  current = head;

  while (!feof(read)) {
    
    /* Count lines */
    j++; 
  
    line = fgets(l, CONFIG_MAX_LINE_LENGTH, read);
    
    /* Skip blank lines and any NULL */
    if (line == NULL) break;
    if (line[0] == '\n') continue;
    
    /* Now we have to trim leading and trailing spaces etc */
    line = ConfigTrim(line);

    /* Check for full line comments and skip them */
    if (strspn(line, COMM) > 0) continue;
    
    /* Check for the separator at the beginning */
    if (strspn(line, SEP) > 0) {
      ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_MISSING_VAR); 
      goto failure;
    }

    /* First split var/value and inline comments.
     * Trim leading and trailing spaces */
    b = ConfigTrim(strtok(line,COMM));

    /* Check for namespaces */
    if (b[0] == '[') {
      if (b[strlen(b)-1] != ']') {
        ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_MISSING_BRACKET); 
        goto failure;
      }

      b = ConfigNameTrim(b);
			
			nextNM = ConfigFindNamespace(b, head);
			
			if (nextNM == NULL) {
				ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_UNKNOWN_NAMESPACE);
				goto failure;
			} else {
				current = nextNM;
			}
      
      n++;

      continue;
    }
  
    /* If no namespace was specified return failure */
    if (current == NULL) {
      ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_NONAMESPACE);
      goto failure;
    }

    /* Check if in the var/value string the separator exist.*/
    if (strstr(b,SEP) == NULL) {
      ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_MISSING_SEP); 
      goto failure;
    }
    
    /* Check some special case:
     * we have separator, but no value */
    if ((strlen(b) - 1) == strcspn(b,SEP)) {
      ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_MISSING_VAL); 
      goto failure;
    }

    /* We allow to have only one separator in line */
    sepc = ConfigCharCount(b, SEP);
    if (sepc > 1) {
      ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_TOOMANY_SEP); 
      goto failure;
    }
    
    /* Ok, now we are prepared */
    c = ConfigTrim(strtok(b,SEP));

		newOP = ConfigFindOption(c, current);
		
		if (newOP == NULL) {
      ConfigMessage(j, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_UNKNOWN_VAR); 
      goto failure;
		}

    while (c!=NULL) {
      if (c[0] == '\n') break;

      valuelen = strlen(c);
			value = calloc(valuelen+sizeof(char),sizeof(char));
      if (!value) {
        perror("LRC_ASCIIParser: line 542 alloc failed");
        goto failure;
      }
			strncpy(value, c, valuelen);
			value[valuelen] = CONFIG_NULL;

			strncpy(newOP->value, value, valuelen);
			newOP->value[valuelen] = CONFIG_NULL;
			
			c = ConfigTrim(strtok(NULL,"\n"));

			if (value) free(value);
    }  
		
  }
  return n;

failure:
  return -1;
}

/**
 * @fn void ConfigCleanup(configNamespace* head)
 * @brief Cleanup assign pointers. This is required for proper memory managment.
 */
void ConfigCleanup(configNamespace* head){

  config* currentOP = NULL;
  config* nextOP = NULL;
  configNamespace* nextNM = NULL;
  configNamespace* current = NULL;

  current = head;

  while (current) {
    nextNM = current->next;
    currentOP = current->options;
      
    while (currentOP) {
      nextOP = currentOP->next;
      if (currentOP) free(currentOP);
      currentOP = nextOP;
    }

    if (current) free(current);
    current=nextNM;
    
  }

  head = NULL;
}

/**
 * HDF5 parser 
 * 
 * @fn int LRC_HDF5Parser(hid_t file, configNamespace *head)
 * @brief Parse config data stored in HDF5 files.
 *
 * @param file
 *   The handler of the file.
 *
 * @param head
 *   Pointer to the structure with default values.
 *
 * @return
 *   Number of read namespaces or -1 on failure
 *
 * @todo
 *   - Open, rather than recreate compound datatype.
 *
 */
int ConfigHDF5Parser(hid_t file, char* group_name, configNamespace* head){
  
  hid_t master_group, group, dataset, dataspace;
  hid_t ccm_tid, name_dt, value_dt;
  herr_t status;
  H5G_info_t group_info;

  int numOfNM = 0, i = 0, k = 0;
  char link_name[CONFIG_MAX_LINE_LENGTH];
  ssize_t vlen, lname;
  char tname[CONFIG_LEN];
  hsize_t edims[1], emaxdims[1];

  configNamespace* nextNM = NULL;
  config* newOP = NULL;
  configNamespace* current = NULL;

  char value[CONFIG_LEN];
  ccd_t* rdata = NULL;

  /* For future me: how to open compound data type and read it,
   * without rebuilding memtype? Is this possible? */

  /* Create variable length string datatype */
  name_dt = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(name_dt, CONFIG_LEN);
  if (status < 0) goto failure;

  value_dt = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(value_dt, CONFIG_LEN);
  if (status < 0) goto failure;

  /* Create compound datatype for the memory */
  ccm_tid = H5Tcreate(H5T_COMPOUND, sizeof(ccd_t));

  H5Tinsert(ccm_tid, "Name", HOFFSET(ccd_t, name), name_dt);
  H5Tinsert(ccm_tid, "Value", HOFFSET(ccd_t, value), value_dt);
  H5Tinsert(ccm_tid, "Type", HOFFSET(ccd_t, type), H5T_NATIVE_INT);

  /* Open config group */
  master_group = H5Gopen(file, CONFIG_GROUP, H5P_DEFAULT);
  group = H5Gopen(master_group, group_name, H5P_DEFAULT);

  /* Get group info */
  status = H5Gget_info(group, &group_info);
  if (status < 0) goto failure;

  /* Get number of opts (dataspaces) */
  numOfNM = group_info.nlinks;

  /* Iterate each dataspace and assign config values */
  for (i = 0; i < numOfNM; i++) {

    /* Get name of dataspace -> the namespace */
    H5Lget_name_by_idx(group, ".", H5_INDEX_NAME, H5_ITER_INC, i, 
      link_name, CONFIG_MAX_LINE_LENGTH, H5P_DEFAULT);

    /* Get size of the table with config data */
    dataset = H5Dopen(group, link_name, H5P_DEFAULT);
    dataspace = H5Dget_space(dataset);
    H5Sget_simple_extent_dims(dataspace, edims, emaxdims);
    
    /* We will get all data first */
    rdata = calloc(((int)edims[0])*sizeof(ccd_t), sizeof(ccd_t));
    if (!rdata) {
      perror("LRC_HDFParser: line 682 alloc failed");
      goto failure;
    }
    
    status = H5Dread(dataset, ccm_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
    if (status < 0) goto failure;

    /* Check if namespace exists */
    nextNM = ConfigFindNamespace(link_name, head);
    if (nextNM == NULL) {
				ConfigMessage(i, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_UNKNOWN_NAMESPACE);
        goto failure;
    } else {
        current = nextNM;
    }

    /* Assign values */
    for (k = 0; k < (int)edims[0]; k++) {
      
      /* Find option and change the value */
      lname = strlen(rdata[k].name);
      strncpy(tname, rdata[k].name, lname);
      tname[lname] = CONFIG_NULL;
      
      newOP = ConfigFindOption(tname, current);

      if (newOP == NULL) {
        ConfigMessage(i, LRC_ERR_CONFIG_SYNTAX, LRC_MSG_UNKNOWN_VAR); 
        goto failure;
      }

      vlen = strlen(rdata[k].value);
      strncpy(value, rdata[k].value, vlen);
      value[vlen] = CONFIG_NULL;

      strncpy(newOP->value, value, vlen);
      newOP->value[vlen] = CONFIG_NULL;
    
      newOP->type = rdata[k].type;
    }

    status = H5Dvlen_reclaim(ccm_tid, dataspace, H5P_DEFAULT, rdata);
    if (status < 0) goto failure;
    
    status = H5Sclose(dataspace);
    if (status < 0) goto failure;
    
    status = H5Dclose(dataset);
    if (status < 0) goto failure;

    free(rdata);
  }

  status = H5Tclose(ccm_tid);
  if (status < 0) goto failure;
  
  status = H5Tclose(name_dt);
  if (status < 0) goto failure;
  
  status = H5Tclose(value_dt);
  if (status < 0) goto failure;

  status = H5Gclose(group);
  if (status < 0) goto failure;
  
  status = H5Gclose(master_group);
  if (status < 0) goto failure;
 
  return numOfNM;

failure:
  return -1;
}

/**
 * @}
 */

/**
 * @fn void LRC_ASCIIWriter(FILE*, char* sep, char* comm, configNamespace* head)
 * @brief Write ASCII config file.
 * @return
 *  Should return 0 on success, errcode otherwise
 */
int ConfigAsciiWriter(FILE* write, char* sep, char* comm, configNamespace* head){

  config* currentOP = NULL;
  config* nextOP = NULL;
  configNamespace* nextNM = NULL;
  configNamespace* current = NULL;

  if (!head) {
    perror("LRC_ASCIIWriter: no config assigned");
    return -1;
  }

  current = head;

  fprintf(write,"%s Written by LibReadConfig \n",comm);

  do {
    if (current) {
      fprintf(write, "[%s]\n",current->space);
      currentOP = current->options; 
      do {
        if (currentOP) {
          fprintf(write, "%s %s %s\n", currentOP->name, sep, currentOP->value);
          nextOP = currentOP->next;
          currentOP = nextOP;
        }
      } while(currentOP);
      
      nextNM = current->next;
      current = nextNM;
    }
    fprintf(write,"\n");
  } while(current);

  fprintf(write,"\n");
  return 0;
}

/**
 * @fn void LRC_HDF5writer(hid_t file, configNamespace* head)
 * @brief Write config values to hdf file.
 * 
 * @param file
 *   The handler of the file.
 *
 * @return
 *  Should return 0 on success, errcode otherwise
 *
 */
int ConfigHDF5Writer(hid_t file, char* group_name, configNamespace* head){

  hid_t master_group, group, dataset, dataspace, memspace;
  hid_t ccm_tid, ccf_tid, name_dt, value_dt;
  hsize_t dims[2], dimsm[2], offset[2], count[2], stride[2];
  herr_t status;
  htri_t cctt;
  int k = 0;
  size_t nlen, vlen;

  config* currentOP = NULL;
  config* nextOP = NULL;
  configNamespace* nextNM = NULL;
  configNamespace* current = NULL;

  ccd_t* ccd;
  ccd = calloc(sizeof(ccd_t),sizeof(ccd_t));
  if (!ccd) {
    perror("LRC_HDFWriter: line 851 malloc failed");
    goto failure;
  }

  cctt = H5Lexists(file, CONFIG_GROUP, H5P_DEFAULT);
  if (!cctt) {
    master_group = H5Gcreate(file, CONFIG_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  } else {
    master_group = H5Gopen(file, CONFIG_GROUP, H5P_DEFAULT);
  }
  group = H5Gcreate(master_group, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  if (!head) {
    perror("LRC_HDFWriter: no config assigned");
    goto failure;
  }

  current = head;

  /* Create variable length string datatype */
  name_dt = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(name_dt, CONFIG_LEN);
  if (status < 0) goto failure;

  value_dt = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(value_dt, CONFIG_LEN);
  if (status < 0) goto failure;

  /* Create compound datatype for the memory */
  ccm_tid = H5Tcreate(H5T_COMPOUND, sizeof(ccd_t));

  H5Tinsert(ccm_tid, "Name", HOFFSET(ccd_t, name), name_dt);
  H5Tinsert(ccm_tid, "Value", HOFFSET(ccd_t, value), value_dt);
  H5Tinsert(ccm_tid, "Type", HOFFSET(ccd_t, type), H5T_NATIVE_INT);

  /* Create compound datatype for the file */
  ccf_tid = H5Tcreate(H5T_COMPOUND, 8 + 2*CONFIG_LEN);

  status = H5Tinsert(ccf_tid, "Name", 0, name_dt);
  if (status < 0) goto failure;
  
  status = H5Tinsert(ccf_tid, "Value", CONFIG_LEN, value_dt);
  if (status < 0) goto failure;
  
  status = H5Tinsert(ccf_tid, "Type", 2*CONFIG_LEN,H5T_NATIVE_INT);
  if (status < 0) goto failure;

  /* Commit datatype */
  cctt = H5Lexists(file, CONFIG_HDF5_DATATYPE, H5P_DEFAULT);
  if (!cctt) {
    status = H5Tcommit(file, CONFIG_HDF5_DATATYPE, ccf_tid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (status < 0) goto failure;
  }

  do {
    if (current) { 

      currentOP = current->options;
      dims[0] = (hsize_t)ConfigCountOptions(current->space, current);
      dims[1] = 1;
      dataspace = H5Screate_simple(1, dims, NULL);
    
      dataset = H5Dcreate(group, current->space, ccf_tid, dataspace, 
          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* Write config data one by one in given namespace */
    k = 0;
    do {
      if (currentOP) { 

      /* Assign values */
      nlen = strlen(currentOP->name);
      strncpy(ccd->name, currentOP->name, nlen);
			ccd->name[nlen] = CONFIG_NULL;
      
      vlen = strlen(currentOP->value);
      strncpy(ccd->value, currentOP->value, vlen);
			ccd->value[vlen] = CONFIG_NULL;
      
      ccd->type = currentOP->type;

      /* Prepare HDF write */
      offset[0] = k; 
      offset[1] = 0;
      
      dimsm[0] = 1; 
      dimsm[1] = 1;
      
      count[0] = 1; 
      count[1] = 1;
      
      stride[0] = 1;
      stride[1] = 1;

      memspace = H5Screate_simple(1, dimsm, NULL);
      status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, stride, count, NULL);
      if (status < 0) goto failure;

      status = H5Dwrite(dataset, ccm_tid, memspace, dataspace, H5P_DEFAULT, ccd);
      if (status < 0) goto failure;
      
      status = H5Sclose(memspace);
      if (status < 0) goto failure;

      nextOP = currentOP->next;
      currentOP = nextOP;
      k++;

      }
    } while(currentOP);

    status = H5Dclose(dataset);
    if (status < 0) goto failure;
    
    status = H5Sclose(dataspace);
    if (status < 0) goto failure;

    nextNM = current->next;
    current = nextNM;
    }
  } while(current);

  status = H5Gclose(group);
  if (status < 0) goto failure;
  
  status = H5Gclose(master_group);
  if (status < 0) goto failure;
  
  status = H5Tclose(name_dt);
  if (status < 0) goto failure;
  
  status = H5Tclose(value_dt);
  if (status < 0) goto failure;
  
  status = H5Tclose(ccf_tid);
  if (status < 0) goto failure;
  
  status = H5Tclose(ccm_tid);
  if (status < 0) goto failure;

  free(ccd);

  return 0;
failure:
  return -1;
}

/**
 * @fn void LRC_printAll(configNamespace* head)
 * @brief Prints all options.
 */
void ConfigPrintAll(configNamespace* head){

  config* currentOP = NULL;
  config* nextOP = NULL;
  configNamespace* nextNM = NULL;
  configNamespace* current = NULL;

  if (head) current = head;

  do {
    if (current) {
      nextNM = current->next;
      printf("[%s][%d]\n",current->space, ConfigCountOptions(current->space, current));
      currentOP = current->options;
      do {
        if (currentOP) {
          nextOP = currentOP->next;
          printf("%s = %s [type %d]\n", currentOP->name, currentOP->value, currentOP->type);
          currentOP = nextOP;
        }
      } while(currentOP);
      
      current=nextNM;
    }
		printf("\n");
    
  } while(current);

  current = NULL;

}

/**
 * @fn int ConfigAssignDefaults(options* cd)
 * @brief Assign default values
 *
 * @return
 *  Should return 0 on success, errcode otherwise
 */

configNamespace* ConfigAssignDefaults(options* cd){

  configNamespace* nextNM = NULL;
  configNamespace* current = NULL;
  configNamespace* head = NULL;
  config* newOP = NULL;
  config* nextOP = NULL;
  config* currentOP = NULL;
  size_t slen, nlen, vlen;
  char space[CONFIG_LEN]; 
  char name[CONFIG_LEN]; 
  char value[CONFIG_LEN]; 

  int i = 0;

  head = NULL;
  current = NULL;

  while (cd[i].space[0] != CONFIG_NULL) {

      /* Prepare namespace */
      slen = strlen(cd[i].space);
      strncpy(space,cd[i].space, slen);
			space[slen] = CONFIG_NULL;

      if (head == NULL) {
        head = ConfigNewNamespace(space);
        current = head;
      } else {
        nextNM = ConfigFindNamespace(space, head);

        if (nextNM == NULL) {
          current = ConfigLastLeaf(head);
          current->next = ConfigNewNamespace(space);
          current = current->next;
        } else {
				  current = nextNM;
          current->next = NULL;
			  }
      }

      if (current) { 

				/* Prepare var name*/
       	nlen = strlen(cd[i].name);
       	strncpy(name, cd[i].name, nlen);
				name[nlen] = CONFIG_NULL;

				currentOP = ConfigFindOption(name, current);

        if (currentOP == NULL) {  	
					newOP = calloc(sizeof(config), sizeof(config));
          if (!newOP) {
            perror("ConfigAssignDefaults: line 1108 alloc failed");
            return NULL;
          }
          newOP->type = C_INT;
          newOP->next = NULL;
				
				  if (current->options == NULL) {
         	  current->options = newOP;
            current->options->next = NULL;
					  currentOP = current->options;
				  } else {
					  currentOP = current->options;
            nextOP = currentOP->next;
					  if (nextOP) {
              do {
                if (!nextOP->next) currentOP = nextOP;
                nextOP = nextOP->next;
              } while (nextOP);
					  }
					  currentOP->next = newOP;
					  currentOP = newOP;
				  }
				
       	  strncpy(currentOP->name, name, nlen);
				  currentOP->name[nlen] = CONFIG_NULL;
				
          /* Prepare the value */
          vlen = strlen(cd[i].value);
          strncpy(value, cd[i].value, vlen);
          value[vlen] = CONFIG_NULL;
            
          strncpy(currentOP->value, value, vlen);
          currentOP->value[vlen] = CONFIG_NULL;
        
          /* Assign type */
          currentOP->type = cd[i].type;
				} else {
          ConfigModifyOption(current->space, currentOP->name, cd[i].value, cd[i].type, current);
        }
      }

    i++;
  }

  return head;
}

/**
 * @fn int ConfigAllOptions(configNamespace* head)
 * @brief Count all available options
 *
 * @param head
 * First namespace in the options list
 *
 * @return
 * Number of options (all available namespaces)
 */
int ConfigAllOptions(configNamespace* head) {
  int allopts = 0;
  config* currentOP = NULL;
  config* nextOP = NULL;
  configNamespace* nextNM = NULL;
  configNamespace* current = NULL;

  if (head) current = head;

  do {
    if (current) {
      nextNM = current->next;
      currentOP = current->options;
      do {
        if (currentOP) {
          allopts++;
          nextOP = currentOP->next;
          currentOP = nextOP;
        }
      } while(currentOP);
      current=nextNM;
    }
  } while(current);


  return allopts;
}

/**
 * @fn configNamespace* ConfigFindNamespace(char* namespace)
 * @brief Search for given namespace
 *
 * @param namespace
 *  The name of the namespace to be searched for
 *
 * @return
 *  Pointer to the namespace or NULL if namespace was not found
 */
configNamespace* ConfigFindNamespace(char* namespace, configNamespace* head){
  
  configNamespace* test = NULL;
  
  if (head && namespace) {
    test = head;

    while (test) {
      if (strcmp(test->space, namespace) == 0) {
        return test;
      }
      test = test->next;
    }
  }
  return NULL;
}

configNamespace* ConfigLastLeaf(configNamespace* head) {

  configNamespace* test = NULL;

  if (head) {
    test = head;

    if (test->next == NULL) return test;

    while (test->next) {
      test = test->next;
      if (test->next == NULL) return test;
    }
  }

  return head;
}

/**
 * @fn config* ConfigFindOption(char* varname, configNamespace* current)
 * @brief Search for given variable
 *
 * @param varname
 *  The name of the variable to be searched for
 *
 * @return
 *  The pointer to the variable or NULL if the variable was not found
 *
 */
config* ConfigFindOption(char* varname, configNamespace* current){

  config* testOP = NULL;
  size_t vlen, olen;
  char var[CONFIG_LEN];
  char opt[CONFIG_LEN];

  if (current && varname) {
    if (current->options) {
      testOP = current->options;
      vlen = strlen(varname);
      strncpy(var, varname, vlen);
      var[vlen] = CONFIG_NULL;

      while (testOP) {
        if (testOP->name) {

          olen = strlen(testOP->name);
          strncpy(opt, testOP->name, olen);
          opt[olen] = CONFIG_NULL;
          
          if (strcmp(opt, var) == 0) {
            return testOP;
          }
        }
        testOP = testOP->next;
      }
    }
  }

  return NULL;
}

/**
 * @fn config* ConfigModifyOption(char* varname, char* newvalue, int newtype, configNamespace* head)
 * @brief Modifies value and type of given option.
 *
 * @return
 *  The pointer to modified option or NULL if option was not found
 */
config* ConfigModifyOption(char* namespace, char* varname, char* newvalue, int newtype, configNamespace* head){
	
	config* option = NULL;
  configNamespace* current = NULL;
	size_t vlen;

  if (head) {
	  current = ConfigFindNamespace(namespace, head);
	  if (current) {
      option = ConfigFindOption(varname, current);

	    vlen = strlen(newvalue);

      if (option) {
        if (strcmp(option->value, newvalue) != 0) {
          strncpy(option->value, newvalue, vlen);
          option->value[vlen] = CONFIG_NULL;
        }
        if (option->type != newtype) {
          option->type = newtype;
        }
      }
    }
  }
	return option;
}

char* ConfigGetOptionValue(char* namespace, char* var, configNamespace* head){

	config* option = NULL;
  configNamespace* current = NULL;

  if (head) {
    current = ConfigFindNamespace(namespace, head);
    if (current) {
      option = ConfigFindOption(var, current);
      if (option && option->value) return option->value;
    }
  }
  return NULL;
}

/**
 * @fn Option2Int(char* namespace, char* varname, configNamespace* head)
 * @brief Converts the option to integer
 *
 * @return
 *  Converted option
 */
int Option2Int(char* namespace, char* varname, configNamespace* head){
  
  config* option = NULL;
  configNamespace* current = NULL;
  int value = 0;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);

      if (option) {
        if (option->value) {
          value = atoi(option->value);
        }
      }
    }
  }

  return value;
}

/**
 * @fn Option2Float(char* namespace, char* varname, configNamespace* head)
 * @brief Converts the option to float
 *
 * @return
 *  Converted option
 */
float Option2Float(char* namespace, char* varname, configNamespace* head){
  
  config* option = NULL;
  configNamespace* current = NULL;
  float value = 0.0;
  char* p = NULL;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);

      if (option) {
        if (option->value) {
          value = strtof(option->value, &p);
        }
      }
    }
  }
  
  return value;
}

/**
 * @fn Option2Double(char* namespace, char* varname, configNamespace* head)
 * @brief Converts the option to double
 *
 * @return
 *  Converted option
 */
double Option2Double(char* namespace, char* varname, configNamespace* head){
  
  config* option = NULL;
  configNamespace* current = NULL;
  double value = 0.0;
  char* p = NULL;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);

      if (option) {
        if (option->value) {
          value = strtod(option->value, &p);
        }
      }
    }
  }
  
  return value;
}

/**
 * @fn Option2LDouble(char* namespace, char* varname, configNamespace* head)
 * @brief Converts the option to long double
 *
 * @return
 *  Converted option
 */
long double Option2LDouble(char* namespace, char* varname, configNamespace* head){
  
  config* option = NULL;
  configNamespace* current = NULL;
  long double value = 0.0;
  char* p = NULL;
  
  if (head && namespace) {
    current = ConfigFindNamespace(namespace, head);
    if (current && varname) {
      option = ConfigFindOption(varname, current);
    
      if (option) {
        if (option->value) {
          value = strtold(option->value, &p);
        }
      }
    }
  }

  return value;
}

/**
 * @fn ConfigCountOptions(char* nm, configNamespace* head)
 * @brief Counts number of options in given namespace
 *
 * @return
 *  Number of options in given namespace, 0 otherwise
 */
int ConfigCountOptions(char* nm, configNamespace* head){

  configNamespace* nspace;
  config* option;
  int opts = 0;

  if (head && nm) {
    nspace = ConfigFindNamespace(nm, head);

    if (nspace) {
      option = nspace->options;
      while (option) {
        opts++;
        option = option->next;
      }
    }
  }

  return opts;
}

/**
 * @fn int ConfigItoa(char* str, char* intval)
 * @brief Implementation of itoa function
 *
 * The function converts numerical values to string. 
 * For using it with integers, try:
 * ConfigItoa(str, (int*)value, C_INT)
 *
 * For using it with floats and doubles, try:
 * LRC_itao(str, &value, LRC_FLOAT/T_DOUBLE) etc.
 *
 * It is not clear for me if this is a good implementation. However, this is the
 * only way I can do it in one function for all types of numerical values.
 *
 * For future me: implement it better, if this is possible in C
 * 
 * @todo
 *  Error checking
 *
 * @return
 *  Should return 0 on success, errcode otherwise
 */
int ConfigItoa(char* str, int val, int type){

  if (type == C_INT) sprintf(str, "%d", val);

/*  if(type == LRC_LONGINT) sprintf(str, "%ld", (long int)val);
  if(type == LRC_FLOAT) sprintf(str, "%f", *(float*)val);
  if(type == T_DOUBLE) sprintf(str, "%lf", *(double*)val);
  if(type == LRC_LONGDOUBLE) sprintf(str, "%Lf", *(long double*)val);
*/
  return 0;
}

/**
 * @}
 */

/**
 * @function
 * Counts the number of all options in the default LRC options structure. The option
 * structure must end with LRC_OPTIONS_END
 */
int ConfigCountDefaultOptions(options *in) {
  int options;

  options = 0;

  while (in[options].space[0] != CONFIG_NULL) {
    options++;
  }

  return options;
}

/**
 * @function
 * Merges two LRC default option structures
 *
 * @param options*
 *  Input structure
 * @param options*
 *  The structure, that we want to merge with the Input one
 *
 * @return
 *  Error code or 0 otherwise. The Input structure is extended with the second one.
 */
int ConfigMergeDefaults(options *in, options *add) {
  int status;
  int index, addopts, i, j, flag;
  status = 0;

  index = ConfigCountDefaultOptions(in);
  addopts = ConfigCountDefaultOptions(add);

  for (i = 0; i < addopts; i++) {
    flag = 0;
    for (j = 0; j < index; j++) {
      if (strcmp(in[j].name,add[i].name) == 0) {
        in[j] = add[i];
        flag = 1;
      } 
    }
    if (flag == 0) in[index + i] = add[i];
  }

  return status;
}

/**
 * @function
 * Counts all options in all namespaces
 */
int ConfigCountAllOptions(configNamespace *head) {
  int opts = 0, allopts = 0;
  configNamespace *current;

  current = head;
  while (current) {
    opts = ConfigCountOptions(current->space, current);
    allopts = allopts + opts;
    current = current->next;
  }
    
  return allopts;
}

/**
 * @function
 * Converts the linked list back to structure
 *
 * @return
 * Dynamically allocated options structure. You must free it.
 */
int ConfigHead2StructNoalloc(configNamespace *head, options *c) {
  int i = 0;
  size_t len;

  config *currentOP = NULL;
  config *nextOP = NULL;
  configNamespace *nextNM = NULL;
  configNamespace *current = NULL;

  if (head) {
    current = head;

    do { 
      if (current) {
        nextNM = current->next;
        currentOP = current->options;
        do { 
          if (currentOP) {
            len = strlen(current->space) + 1; 
            strncpy(c[i].space, current->space, len);
            len = strlen(currentOP->name) + 1; 
            strncpy(c[i].name, currentOP->name, len);
            len = strlen(currentOP->value) + 1; 
            strncpy(c[i].value, currentOP->value, len);
            c[i].type = currentOP->type;
            
            i++; 
            nextOP = currentOP->next;
            currentOP = nextOP;
          }    
        } while(currentOP);
        current = nextNM;
      }    
    } while(current);
  }

  return 0;
}

options* ConfigHead2Struct(configNamespace *head) {
  options *c;
  int opts = 0;
  opts = ConfigAllOptions(head);

  c = calloc(opts*sizeof(options), sizeof(options));
  
  ConfigHead2StructNoalloc(head, c);
  return c;
}

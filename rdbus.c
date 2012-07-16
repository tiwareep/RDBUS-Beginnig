 /**
 * rdbus.c
 *   A D-bus Implementation for Racket.
 */

// +---------+--------------------------------------------------------
// | Headers |
// +---------+

#include <gio/gio.h>
#include "rdbus.h"
#include <escheme.h>


// +---------+--------------------------------------------------------
// | Globals |
// +---------+

//GDBusType *SESSION_BUS = NULL;
GDBusProxy *Proxyobj = NULL;

// +-----------+------------------------------------------------------
// | Constants |
// +-----------+

/**
 * The maximum number of live objects our program supports.  In the
 * long term, this should be an arbitrary number.
 */
#define MAX_OBJECTS 128


// +--------------------+---------------------------------------------
// | Predeclarations |
// +--------------------+

char * tostring (Scheme_Object *obj);

// +--------------------+---------------------------------------------
// | Exported Functions |
// +--------------------+

void
rdbus_init (void)
{
 
  // GDBusProxyFlags G_DBUS_PROXY_NONE;
  // we do not know. It's a struct and it has two ** in the API. 
 //it provides the facilities for registering and managing all fundamental data types, user-defined object and interface types. 
  g_type_init ();
   
  //creates a proxy for accessing interface_name on the remote object at object_path owned by name at connection and synchronously loads.  
  
} // rdbus_init ()

/**
 * Get a new object for a particular service, object, and interface.  Returns
 * an integer used to refer to that object.  If we cannot create the object,
 * returns a negative number.
 */
int 
rdbus_get_object (const gchar *name, const gchar *object_path, const gchar *interface_name)
{
  const GDBusProxy *objects[MAX_OBJECTS];   // The objects we've allocated
  const int latest_object = 0;              // The index of the latest object

  GError *error;
  
  error = NULL;
  Proxyobj =  g_dbus_proxy_new_for_bus_sync ( G_BUS_TYPE_SESSION, 
					      G_DBUS_PROXY_FLAGS_NONE,
					      NULL,
					      name,
					      object_path,
					      interface_name,
					      NULL,
					      &error);

  // Check the result
  if ((Proxyobj == NULL) || (error != NULL))
    {
      // Values less than zero signify an error.
      return -1; 
    }

  return 0;
} // rdbus_get_object

GVariant *
scheme_obj_to_gvariant (Scheme_Object *list)
{
 
  GVariant *rvalue;
  Scheme_Object *firstelement;
  int length;
  long i;
  char* rstring;
  double rdouble;

  rvalue = NULL; 
  length = scheme_list_length (list);
  if (length == 0)
    {
      return rvalue ;
    }  
  
  else if (length == 1)
    {
      // Get the first element of the argument
      firstelement = scheme_car (list);
 
      // checking the scheme_type to see whether it is an integer or not
      // Eventually see if we can convert this to a switch statement.
      if (SCHEME_TYPE (firstelement)== scheme_integer_type)
	{
	  // we saved the return value at &i
	  scheme_get_int_val (list,&i);
	  // we concert it to g_variant
          rvalue = g_variant_new ("(i)", i);
	  return rvalue;
	} // if it's an integer
      else if (SCHEME_TYPE (firstelement) == scheme_char_type)
	{
	  //getting the string out of the scheme_object
	  rstring = SCHEME_BYTE_STR_VAL(list);
	  // we will convert it to g_variant
	  rvalue = g_variant_new_string(rstring);
	  return rvalue;
	} // if it's a character
      else if (SCHEME_TYPE (firstelement) == scheme_double_type)
	{
	  //getting the double out of the scheme_object
	  rdouble = scheme_real_to_double(list);
	  // we will convert it to g_variant
	  rvalue = g_variant_new_double(rdouble);
	  return rvalue;
	} // if it's a double
    } // if we have a single element
 
  return rvalue;
} // scheme_obj_to_gvariant

Scheme_Object *
gvariant_to_schemeobj (GVariant *ivalue)
{
  Scheme_Object *fvalue = NULL;
  const gchar *fstring;
  gsize length;
  gsize *plength;
  gint64 r1;
  

  length = g_variant_get_size(ivalue);
  plength = &length;

  if (g_variant_is_of_type (ivalue, G_VARIANT_TYPE_INT64))
    { 
      g_variant_get (ivalue,"(i)", &r1);
      fvalue = scheme_make_integer_value(r1);
      return fvalue;
    }

  else if (g_variant_is_of_type (ivalue,G_VARIANT_TYPE_STRING))
    {
      fstring = g_variant_get_string (ivalue, plength);
      fvalue = scheme_make_utf8_string (fstring);
      return fvalue;
    }

  return fvalue;
    
  


}


/**
 * Convert an array of Scheme objects into a list of the same objects.
 */
Scheme_Object *
make_object_list (int n, Scheme_Object *values[])
{
  Scheme_Object *result;  // The result we're building
  int i;                  // Everyone's favorite counter variable
  result = scheme_null;
  // Step through the objects from right to left, adding each to the front
  // of the list.
  for (i = n-1; i >= 0; i--)
    {
      result = scheme_make_pair (values[i], result);
    } // for
  // And we're done.
  return result;
} // make_object_list

Scheme_Object *
rdbus_call_method (int i, Scheme_Object *proc, Scheme_Object *list )
{
  //This is the final return value that will be saved in memory after calling Gimp Server 
  GVariant *frvalue;
  // Intermediary return value for helper method return for scheme to gvariant
  GVariant *ivalue;
  // returning the string of the method name
  const char *methodname;
  //saving the error in memory
  GError *error;
  //final Scheme_Object
  Scheme_Object *fobject;
  //Our GDBusProxy Object
  GDBusProxy *proxy;
  
  
  if (i == 0)
    { 
      proxy = Proxyobj; 
      ivalue = scheme_obj_to_gvariant (list);
      // the method is converted into string here
      methodname = tostring (proc);
      fprintf (stderr, "Calling %s\n", methodname);
      frvalue = g_dbus_proxy_call_sync (proxy, methodname, ivalue, 0, -1, NULL, &error);
      fobject = gvariant_to_schemeobj (frvalue);
      return fobject;
    } // if (i == 0)
 
  else // if (i != 0)
    {
      return scheme_make_utf8_string("There is sth wrong with the Proxy Object");
    } // if (i != 0)
} // rdbus_call_method






/**
 * A wrapper for rdbus_call_method that makes it easier to export our function
 * to Scheme.
 */
Scheme_Object *
pardbus_call_method (int argc, Scheme_Object *argv[])
{
  // Verify that we have the appropriate number of arguments.
  if (argc < 2)
    {
      scheme_signal_error ("Call method needs at least two parameters.");
      return NULL;
    } // if there are insufficiently many arguments available.

  // rdbus_call_method does the heavy lifting
  // Hack!  Right now, we are going to assume zeroary procedures
  return rdbus_call_method (SCHEME_INT_VAL (argv[0]), argv[1], 
                            make_object_list (argc-2, argv+2));
} // pardbus_call_method

/**
 * Determine whether we can convert a Scheme_Object to a string.
 */
int
stringp (Scheme_Object *obj)
{
  return SCHEME_BYTE_STRINGP (obj) || SCHEME_CHAR_STRINGP (obj);
} // stringp

/**
 * Convert a Scheme_Object to a string.  Returns NULL if it fails.
 */
char *
tostring (Scheme_Object *obj)
{
  if (SCHEME_BYTE_STRINGP (obj))
    return SCHEME_BYTE_STR_VAL (obj);
  else if (SCHEME_CHAR_STRINGP (obj))
    return SCHEME_BYTE_STR_VAL (scheme_char_string_to_byte_string (obj));
  else
    return NULL;
} // tostring

/**
 * A wrapper for rdbus_get_object.
 */
Scheme_Object *pardbus_get_object (int argc, Scheme_Object *argv[])
{
  char *service;
  char *path;
  char *interface;
  int object;

  // Verify the number of parameters
  if (argc != 3)
    {
      scheme_signal_error ("rdbus-get-object: Requires exactly three parameters.\n");
      return scheme_void;
    } // if we have the wrong number of parameters

  // Verify the type of the parameters
  if ( (! stringp (argv[0]))
       || (! stringp (argv[1]))
       || (! stringp (argv[2])) )
    {
      scheme_signal_error ("rdbus-get-object: All three parameters must be strings");
    } // if one of the parameters is not a string

  // Extract the parameters
  service = tostring (argv[0]);
  path = tostring (argv[1]);
  interface = tostring (argv[2]);

  // Rely on rdbus_get_object to do the real work
  object = rdbus_get_object (service, path, interface);

  // Sanity check.  rdbus_get_object returns a negative number upon error.
  if (object < 0)   
    {
      scheme_signal_error ("Could not create bus object.");
      return scheme_void;
    } // if (object < 0)

  // We're done.  Return the damn integer.
  return scheme_make_integer (object);
} // pardbus_get_object

/**
 * A wrapper for the rdbus_init
 */

Scheme_Object *pardbus_init (int argc, Scheme_Object *argv[])
{
  rdbus_init();
  return scheme_void;
}//pardbus_init


Scheme_Object *scheme_reload (Scheme_Env *env)
{
  Scheme_Env *menv;
  Scheme_Object *proc1;
  Scheme_Object *proc2;
  Scheme_Object *proc3;
  // Converting a C procedure to a scheme procedure.
  proc1 = scheme_make_prim_w_arity (pardbus_get_object, "rdbus-get-object", 3, -1);
  proc2 = scheme_make_prim_w_arity (pardbus_call_method, "rdbus-call-method", 3,-1);
  proc3 = scheme_make_prim_w_arity (pardbus_init, "rdbus_init", 0,0);

  // Add the new procedures to the shared object for Racket to use
  scheme_add_global ("rdbus-get-object", proc1, env);
  scheme_add_global ("rdbus-call-method", proc2, env);
  scheme_add_global ("rdbus_init", proc3, env);
  
  return scheme_void;
} // scheme_reload
  

Scheme_Object *scheme_initialize(Scheme_Env *env)
{
  /* First load is same as every load: */
  return scheme_reload(env);
}

Scheme_Object *
scheme_module_name ()
{
  /* This extension defines a module named `idmodule': */
  return scheme_intern_symbol ("idmodule");
} // scheme_module_name

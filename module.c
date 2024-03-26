#include <emacs-module.h>
#include <libguile.h>
#include <string.h>

int plugin_is_GPL_compatible;
#define lisp_integer(env, integer)              \
  ({                                            \
    emacs_env *_env_ = env;                     \
    _env_->make_integer(_env_, (integer));      \
  })                                            \

#define lisp_string(env, string)                        \
  ({                                                    \
    emacs_env *_env_ = env;                             \
    char* _str_ = string;                               \
    _env_->make_string(_env_, _str_, strlen(_str_));    \
  })

#define lisp_funcall(env, fn_name, ...)                 \
  ({                                                    \
    emacs_env *_env_ = env;                             \
    emacs_value _args_[] = { __VA_ARGS__ };             \
    int _nargs_ = sizeof(_args_) / sizeof(emacs_value); \
    _env_->funcall(_env_,                               \
                   env->intern(env, (fn_name)),         \
                   _nargs_,                             \
                   _args_                               \
                   );                                   \
  })


void *eval(void *n) {
  SCM out = scm_c_eval_string(n);
  SCM output_string_port = scm_open_output_string();
  scm_write(out, output_string_port);
  SCM o = scm_get_output_string(output_string_port);
  return scm_to_utf8_string(o);
}

char *estring_to_string(emacs_env *env, emacs_value lstring) {
  ptrdiff_t strleng;
  env->copy_string_contents(env, lstring, NULL, &strleng);
  char *sdata = malloc(strleng);
  env->copy_string_contents(env, lstring, sdata, &strleng);
  return sdata;
}

SCM eint_to_scm_int(emacs_env *env, emacs_value var) {
  ptrdiff_t strleng;
  intmax_t im=env->extract_integer(env, var);
  return scm_from_intmax(im);
}

emacs_value call(emacs_env *env, ptrdiff_t nargs, emacs_value *args,
                 void *data) {
  emacs_value lstring = args[0];
  char *bbb = scm_with_guile(eval, estring_to_string(env, lstring));
  if (bbb)
    return lisp_string(env,bbb);
  return env->make_integer(env, 1);
}

void *pload(void *n) {
  scm_c_primitive_load(n);
  return SCM_UNSPECIFIED;
}

emacs_value guile_primitive_load(emacs_env *env, ptrdiff_t nargs, emacs_value *args,
                 void *data) {
  emacs_value lstring = args[0];
  scm_with_guile(pload, estring_to_string(env, lstring));
  return env->make_integer(env, 1);
}

void define_elisp_function(emacs_env *env, ptrdiff_t min, ptrdiff_t max,
                           emacs_value (*fun)(emacs_env *env, ptrdiff_t nargs,
                                              emacs_value *args, void *data),
                           char *name) {
  emacs_value func = env->make_function(env, min, max, fun, "", NULL);
  emacs_value symbol = env->intern(env, name);
  /* emacs_value args[] = {symbol, func}; */
  /* env->funcall(env, env->intern(env, "defalias"), 2, args); */
  lisp_funcall(env,"defalias",symbol, func);
}

SCM emacs_funcall(SCM env_ptr,SCM name,SCM arglength,SCM args){
  emacs_env *env=scm_to_pointer(env_ptr);
  ptrdiff_t leng=scm_to_ptrdiff_t(arglength);
  env->funcall(env, env->intern(env, scm_to_utf8_string(name)),0,NULL);
  return SCM_UNSPECIFIED;
};

SCM emacs_message(SCM env_ptr){
  emacs_env *env=scm_to_pointer(env_ptr);
  /* char* str="hello"; */
  /* emacs_value args[] = {env->make_string(env ,str,strlen(str))}; */
  /* env->funcall(env, */
  /*              env->intern(env, "message"), */
  /*              1, */
  /*              args);
   */
  lisp_funcall(env,
             "kill-emacs",
             /* lisp_string(env, "(1+ %d) is %d"), */
               /* (lisp_integer(env, 1)), */
             /* lisp_funcall(env, "1+", lisp_integer(env, 1)) */);
  return SCM_UNSPECIFIED;
};

SCM emacs_env_intern(SCM env_ptr,SCM str){
  emacs_env *env=scm_to_pointer(env_ptr);
  return scm_from_pointer(env->intern(env,scm_to_utf8_string(str)), NULL);;
};

SCM emacs_integer(SCM env_ptr,SCM i){
  emacs_env *env=scm_to_pointer(env_ptr);
  return scm_from_pointer(env->make_integer(env,scm_to_intmax(i)), NULL);;
};

SCM emacs_eq(SCM env_ptr,SCM a,SCM b){
  emacs_env *env=scm_to_pointer(env_ptr);
  emacs_value avalue=scm_to_pointer(a);
  emacs_value bvalue=scm_to_pointer(b);
  return scm_from_bool(env->eq(env,avalue,avalue));;
};

void* init_guile_procs(void* env) {
  SCM module=scm_c_resolve_module("emacs");
  scm_set_current_module(module);
  scm_c_use_module("guile");
  /* scm_c_use_module("srfi srfi-9"); */
  scm_c_module_define(module, "%emacs-env-ptr", scm_from_pointer(env, NULL));
  scm_c_define_gsubr ("%emacs-funcall", 4, 0, 0, (scm_t_subr) emacs_funcall);;
  scm_c_define_gsubr ("%emacs-message", 1, 0, 0, (scm_t_subr) emacs_message);;
  scm_c_define_gsubr ("%emacs-env-intern", 2, 0, 0, (scm_t_subr) emacs_env_intern);;
  scm_c_define_gsubr ("%emacs-integer", 2, 0, 0, (scm_t_subr) emacs_integer);;
  scm_c_define_gsubr ("%emacs-eq", 3, 0, 0, (scm_t_subr) emacs_eq);;
  scm_c_primitive_load("boot.scm");
  return NULL;
}

int emacs_module_init(struct emacs_runtime *ert) {
  emacs_env *env = ert->get_environment(ert);
  define_elisp_function(env, 1, 1, call, "guile-eval-string");
  define_elisp_function(env, 1, 1, guile_primitive_load, "guile-primitive-load");
  scm_with_guile(init_guile_procs,env);
  return 0;
}

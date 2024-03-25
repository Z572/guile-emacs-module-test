#include <emacs-module.h>
#include <libguile.h>
#include <string.h>

int plugin_is_GPL_compatible;

void* eval(void *n) {
  SCM out=scm_c_eval_string(n);
  SCM output_string_port=scm_open_output_string();
  scm_write(out, output_string_port);
  SCM o= scm_get_output_string(output_string_port);
  return scm_to_utf8_string(o);
}

emacs_value call(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data) {
  ptrdiff_t strleng;
  emacs_value lstring = args[0];
  env->copy_string_contents(env, lstring, NULL, &strleng);
  char* sdata = malloc(strleng);
  env->copy_string_contents(env, lstring, sdata, &strleng);
  char* bbb=scm_with_guile(eval,sdata);
  if (bbb)
    return env->make_string(env,bbb , strlen(bbb));
  return env->make_integer(env,1);
}

void define_elisp_function(emacs_env *env) {
  emacs_value func = env->make_function (env, 1, 1, call, "", NULL);
  emacs_value symbol = env->intern (env, "guile-eval-string");
  emacs_value args[] = {symbol, func};
  env->funcall (env, env->intern (env, "defalias"), 2, args);
}

int emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment(ert);
  define_elisp_function(env);
  return 0;
}


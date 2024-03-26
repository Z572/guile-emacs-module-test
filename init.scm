(pk "hello-world!")
(pk (current-module))
(use-modules (srfi srfi-9))
(define-record-type <emacs-env>
  (make-emacs-env ptr)
  emacs-env?
  (ptr emacs-env-ptr))
(define emacs-env (make-emacs-env %emacs-env-ptr))
(pk emacs-env)

;; evaluar desde emacs como cualquier sexp de elisp
;; para que `projectile` tome como raiz cada modulo,
;; y `clangd` reconozca los `include` de archivos de cabecera en los archivos fuente .c
(setq projectile-project-root-functions '(projectile-root-local
                                          projectile-root-top-down
                                          projectile-root-top-down-recurring
                                          projectile-root-bottom-up))

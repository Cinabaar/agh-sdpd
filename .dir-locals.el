((nil . (
         (eval . (progn
                   (require 'projectile)
                   (puthash (projectile-project-root)
                           (format "./compile.sh %s" (projectile-project-root)) 
                           projectile-compilation-cmd-map)))
         (eval . (progn
                   (require 'projectile)
                   (puthash (projectile-project-root)
                            (format "./run.sh %s" (projectile-project-root))
                            projectile-test-cmd-map))))))


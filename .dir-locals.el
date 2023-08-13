((c++-mode . ((compile-command . "cbuild -s build")
              (eval . (progn
                        (flycheck-mode)
                        (setq flycheck-clang-args '("-std=c++20"
                                                    "-Wno-allocaCalled"
                                                    "-Wno-unused-function"
                                                    "-Wno-unused-parameter"
                                                    "-Wno-unused-variable"
                                                    "-Wno-missing-field-initializers"))
                        (setq flycheck-cppcheck-suppressions '("noExplicitConstructor"))
                        (setq flycheck-clang-include-path (list "-I ." (format "-I %s" (4l/project-root))))
                        (setq flycheck-clang-definitions '("-DPLATFORM_WIN32" "-DPLATFORM_X64")))))))

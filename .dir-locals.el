((c++-mode . ((compile-command . "cbuild build")
              (eval . (progn
                        (flycheck-mode)
                        (setq flycheck-clang-args '("-std=c++20"
                                                    "-DPLATFORM_WIN32"
                                                    "-DPLATFORM_X64"
                                                    "-I ."
                                                    "-I .."
                                                    "-Wno-allocaCalled"
                                                    "-Wno-unused-function"
                                                    "-Wno-unused-parameter"
                                                    "-Wno-unused-variable"
                                                    "-Wno-missing-field-initializers"))
                        )))))

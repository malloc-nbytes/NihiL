let () =
  let path = "1.in" in
  let src = Io.load_file path in
  let l = Lexer.lex path src in
  let _ = Lexer.dump l in
  let _ = Parser.parse l in
  ()

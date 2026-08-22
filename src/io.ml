let load_file fp =
  let ch = open_in_bin fp in
  let s = really_input_string ch (in_channel_length ch) in
  close_in ch;
  s

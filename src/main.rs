use std::{path::Path, process::{Command, Stdio}, cmp::min};
use bitflags::bitflags;

use color_print::{cformat, cprintln};

const LINES_TO_SHOW : &str = "2,11p";

const USE_DEFAULT_RANGE : u32 = 0x1;
const USE_CUSTOM_TAIL : u32 = 0x2;
const USE_CUSTOM_RANGE : u32 = 0x4;

enum Unit
{
    B,
    KB,
    MB,
    GB,
    TB,
}

bitflags! {
    struct CmdOptions : u32 {
        const NONE = 0x0;
        const LIST_ALL_FILES = 0x1;
    }
}

fn main() 
{
    let mut args = std::env::args().collect::<Vec<String>>();

    let cmd_flag = parse_options(&mut args);

    if args.len() < 2 {
        elog("no directory specified");
        std::process::exit(1);
    }

    let directory = &args[1];

    if !Path::new(directory).is_dir() {
        elog(&format!("'{}' is not a directory", directory));
        std::process::exit(1);
    }
    let mut flag = USE_DEFAULT_RANGE;
    let mut sed_cmd_str = String::new();
    let mut head_tail = (0, 0);

    // check if a number or range is specified, if so we overwrite
    // the default range
    if args.len() >= 3 {
        sed_cmd_str = match args[2].trim().parse::<usize>() {
            Ok(n) => {
                head_tail.1 = n;
                flag = USE_CUSTOM_TAIL;
                "2,".to_string() + &(n + 1).to_string() + "p"
            },
            Err(_) => {
                head_tail = retrieve_head_tail(&args[2]);
                flag = USE_CUSTOM_RANGE;
                head_tail.0.to_string() + "," + &head_tail.1.to_string() + "p"
            }
        };
    }
    let du_options = match cmd_flag {
        CmdOptions::LIST_ALL_FILES => "-ab",
        _ => "-b"
    };

    // du will list the paths their sizes
    let du = Command::new("du")
                            .args([du_options, directory])
                            .stdout(Stdio::piped())
                            .spawn()
                            .expect("Failed to execute 'du'");

    // self explanatory
    let sort = Command::new("sort")
                              .arg("-n")
                              .stdin(du.stdout.unwrap())
                              .stdout(Stdio::piped())
                              .spawn()
                              .expect("Failed to execute 'sort'");

    // tac will reverse the order of lines
    let tac = Command::new("tac")
                             .stdin(sort.stdout.unwrap())
                             .stdout(Stdio::piped())
                             .spawn()
                             .expect("Failed to execute 'tac'");

    // now we just use sed to filter the lines
    let output = match flag {
        USE_DEFAULT_RANGE => {
            Command::new("sed")
                    .args(["-n", LINES_TO_SHOW])
                    .stdin(tac.stdout.unwrap())
                    .output()
                    .expect("Failed to execute 'sed'")
        },
        _ => {
            Command::new("sed")
                    .args(["-n", &sed_cmd_str])
                    .stdin(tac.stdout.unwrap())
                    .output()
                    .expect("Failed to execute 'sed'")
        },
    };
            
    if !output.status.success() {
        elog(&format!("{}\nlrgest encountered an unexpected error. Terminating program.", 
                 String::from_utf8_lossy(&output.stderr)));
        std::process::exit(1);
    }
    let output_text = String::from_utf8_lossy(&output.stdout);
    let lines = output_text.lines().collect::<Vec<&str>>();

    // if a custom range is used, we need to make sure that tail is not
    // above the number of lines as that wouldn't make any sense
    head_tail.1 = match flag {
        USE_CUSTOM_RANGE => min(lines.len(), head_tail.1 - 1),
        _ => lines.len(),
    };
    if flag == USE_CUSTOM_RANGE {
        head_tail.0 = head_tail.0 - 2;
    }

    for i in 0..lines.len() {
        let split = lines[lines.len() - i - 1].split_whitespace().collect::<Vec<&str>>();
        let path = split[1];
        let size = split[0].parse::<usize>().unwrap();
        let size_with_unit = get_size_with_unit(size);

        cprintln!("{}) <b!>\"{}\"</b!>: {}", head_tail.1 - i + head_tail.0, path, size_with_unit);
    }
    
}

/// Returns a string with the size converted to its
/// respective unit. 'sizeb' should be the size in
/// bytes.
#[inline]
fn get_size_with_unit(sizeb : usize) -> String
{
    let mut unit = Unit::B;
    let mut size = sizeb as f64;

    if size > 1024.0 {
        unit = Unit::KB;
        size /= 1024.0;
    }
    if size > 1024.0 {
        unit = Unit::MB;
        size /= 1024.0;
    }
    if size > 1024.0 {
        unit = Unit::GB;
        size /= 1024.0;
    }
    if size > 1024.0 {
        unit = Unit::TB;
        size /= 1024.0;
    }

    return match unit {
        Unit::B => format!("{:.2}B", size),
        Unit::KB => format!("{:.2}KB", size),
        Unit::MB => format!("{:.2}MB", size),
        Unit::GB => cformat!("<y!>{:.2}GB</y!>", size),
        Unit::TB => cformat!("<r!>{:.2}TB</r!>", size),
    };

}

/// Retrieves head and tail from a string. Will
/// terminate the program if not correctly formatted.
#[inline]
fn retrieve_head_tail(s : &String) -> (usize, usize)
{
    let mut head = String::new();
    let mut tail = String::new();

    let mut encountered_dash = false;

    for c in s.chars() {
        if c == '-' {
            encountered_dash = true;
            continue;
        }

        if c.is_ascii_digit() {
            if encountered_dash {
                tail.push(c);
            } 
            else {
                head.push(c);
            }
        } 
        else {
            elog("invalid range specified");
            std::process::exit(1);
        }
    }

    let head = head.parse::<usize>().unwrap() + 1;
    let tail = tail.parse::<usize>().unwrap() + 1;

    if head > tail {
        elog("invalid range specified");
        std::process::exit(1);
    }

    return (head, tail);
}

/// Prints error message to stderr.
#[inline]
fn elog(msg : &str)
{
    eprintln!("{} {}", cformat!("<s><r>error</r>:</s>"), msg);
}

#[inline]
fn parse_options(args : &mut Vec<String>) -> CmdOptions
{
    let mut flag = CmdOptions::NONE;
    let mut indices_to_rm = Vec::new();

    for (i, arg) in args.iter().enumerate() {
        let mut arg_chars = arg.char_indices();

        if arg_chars.next().unwrap().1 == '-' {
            for c in arg_chars {
                flag |= match c.1 {
                    'a' => CmdOptions::LIST_ALL_FILES,
                    _ => {
                        elog(&format!("invalid option '{}'", c.1));
                        std::process::exit(1);
                    }
                };
            }
            indices_to_rm.push(i);
        }
    }

    for i in indices_to_rm {
        args.remove(i);
    }

    return flag;

}
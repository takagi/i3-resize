extern crate getopts;
extern crate i3ipc;

use std::{env, process};
use getopts::Options;
use i3ipc::I3Connection;
use i3ipc::reply::{Node, NodeLayout};

fn print_usage(program: &str, opts: &Options) {
    let brief = format!("Usage: {} [options]", program);
    print!("{}", opts.usage(&brief));
}

fn get_percent_of_focused_node(layout: NodeLayout, root: &Node) -> Option<f64> {
    fn aux(layout: NodeLayout, root: &Node, percent: Option<f64>) -> Option<f64> {
        if root.focused {
            return percent;
        }

        let Some(&id_of_focused_child) = root.focus.first() else {
            return None;
        };

        let Some(node) = root.nodes.iter().find(|&node| node.id == id_of_focused_child) else {
            panic!("must not be reached")
        };

        if root.layout == layout && root.nodes.len() > 1 {
            return aux(layout, node, node.percent);
        } else {
            return aux(layout, node, percent);
        }
    }

    return aux(layout, root, None);
}

fn resize_window(layout: NodeLayout, threshold: f64, widen_command: &str, narrow_command: &str) {
    let mut conn = I3Connection::connect().unwrap();

    let tree = conn.get_tree().unwrap();

    let Some(percent) = get_percent_of_focused_node(layout, &tree) else {
        return;
    };

    if percent < threshold {
        _ = conn.run_command(widen_command);
    } else {
        _ = conn.run_command(narrow_command);
    }
}

fn resize_window_horizontally() {
    resize_window(NodeLayout::SplitH, 0.67, "resize set width 67ppt", "resize set width 50ppt");
}

fn resize_window_vertically() {
    resize_window(NodeLayout::SplitV, 0.80, "resize set height 80ppt", "resize set height 50ppt");
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let program = args[0].split('/').last().unwrap();

    let mut opts = Options::new();
    opts.optflag("h", "horizontal", "horizontally resize the focused window");
    opts.optflag("v", "vertical", "virtically resize the focused window");

    let matches = opts.parse(&args[1..])
      .unwrap_or_else(|f| panic!("{}", f.to_string()));

    if !matches.opt_present("h") && !matches.opt_present("v") {
        print_usage(program, &opts);
        process::exit(0);
    }

    if matches.opt_present("h") {
        resize_window_horizontally();
    }

    if matches.opt_present("v") {
        resize_window_vertically();
    }
}

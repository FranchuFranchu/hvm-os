#![feature(let_chains, map_try_insert)]

// All returned wires are "unconnected wires"
// that have no wire connected to them yet.
// They are one-sided wires.

// There should also exist *zero-sided wires*.
// How would I implement them?
// Hmm.
use std::{io::Read, collections::BTreeMap, error::Error, iter::Peekable, fmt::Display, cell::RefCell, rc::Rc};

#[derive(Clone, Debug, Default)]
pub struct Addr {
    node: u64,
    port: u64,
}
impl Addr {
    fn get_tuple_3<'a>(&self, t: &'a mut (Port, Port, Port)) -> Option<&'a mut Port> {
        match self.port {
            0 => Some(&mut t.1),
            1 => Some(&mut t.1),
            2 => Some(&mut t.2),
            _ => unreachable!(),
        }
    }
    fn get_tuple_2<'a>(&self, t: &'a mut (Port, Port)) -> &'a mut Port {
        match self.port {
            1 => &mut t.0,
            2 => &mut t.1,
            _ => unreachable!(),
        }
    }
}
impl Display for Addr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}.{}", self.node, self.port)
    }
}
#[derive(Clone, Debug, Default)]
pub struct Port {
    is_primary: bool,
    is_vanilla: bool,
    is_lazy: bool,
    target: Addr,
    label: u64,
}
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct WireId(u64);

impl Display for Port {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Port {{ {}{}{}#{} -> {} }}",
            if self.is_primary { "I" } else { "" },
            if self.is_vanilla { "V" } else { "" },
            if self.is_lazy { "L" } else { "" },
            self.label,
            self.target.clone(),
        )
    }
}
impl Port {
    fn to_bits(&self) -> u64 {
        ( if self.is_primary       { 1 } else { 0 }
        + if self.is_vanilla       { 2 } else { 0 }
        + if self.is_vanilla       { 4 } else { 0 }
        + if self.target.port == 2 { 8 } else { 0 }
        +                          (self.target.node << 4)
        +                          (self.label << 48)
        )
    }
}

#[derive(Default, Clone, Debug)]
pub struct VarData {
    pub binder_count: u64,
    // What you need to connect to
    // to use this var.
    pub to_binder: Option<WireId>,
    pub user_count: u64,
    // What you need to connect to
    // to bind this var.
    pub to_user: Option<WireId>,

    // to_user and to_binder are
    // not connected until the end
}

#[derive(Clone, Debug)]
pub struct NodeData {
    primary: bool,
    vanilla: bool,
    lazy: bool,
    label: u64,
    p0: WireId,
    p1: WireId,
    p2: WireId,
}

pub struct State<R: Iterator<Item = char>, W: std::io::Write> {
    out_stream: W,
    in_stream: Peekable<R>,
    vars: BTreeMap<String, Rc<RefCell<VarData>>>,
    node_data: Vec<NodeData>,
    aliases: BTreeMap<WireId, WireId>,
    label_index: u64,
    node_index: u64,
    label_map: BTreeMap<String, u64>,
    dest_index: u64,
}

pub type Label = u64;
pub type Result<T> = core::result::Result<T, Box<dyn Error>>;

impl<R: Iterator<Item = char>, W: std::io::Write> State<R, W> {
    fn add_node(&mut self,
        primary: bool,
        vanilla: bool,
        lazy: bool,
        label: u64,
    ) -> (WireId, WireId, WireId) {
        self.dest_index += 3;
        self.node_data.push(NodeData {
            primary,
            vanilla,
            lazy,
            label,
            p0: WireId(self.dest_index-3),
            p1: WireId(self.dest_index-2),
            p2: WireId(self.dest_index-1),
        });
        (WireId(self.dest_index-3), WireId(self.dest_index-2), WireId(self.dest_index-1))
    }
    fn is_name_char(c: &char) -> bool {
        !c.is_ascii_whitespace() && !"()[]{}@λ!?;".contains(*c)
    }
    fn parse_whitespace(&mut self) -> Result<()> {
        while let Some(x) = self.in_stream.peek() && x.is_ascii_whitespace() {
            self.in_stream.next();
        }
        Ok(())
    }
    fn parse_name(&mut self) -> Result<String> {
        let mut s = String::new();
        while let Some(x) = self.in_stream.peek() && Self::is_name_char(x) {
            s.push(self.in_stream.next().unwrap())
        }
        Ok(s)
    }
    fn parse_lam(&mut self, binder: bool, label: Label) -> Result<WireId> {
        self.in_stream.next().unwrap();
        let variable = self.parse_term(!binder)?;
        let body = self.parse_term(binder)?;
        let (p0, p1, p2) = self.add_node(true, true, false, label);
        self.connect(&p1, &variable);
        self.connect(&p2, &body);
        Ok(p0)
    }
    fn parse_app(&mut self, binder: bool, label: Label) -> Result<WireId> {
        self.in_stream.next().unwrap();
        let mut term = self.parse_term(binder)?;
        self.skip_whitespace();
        while self.in_stream.peek() != Some(&')') {
            let new_term = self.parse_term(binder)?;
            let (p0, p1, p2) = self.add_node(true, true, false, label);
            self.connect(&term, &p0);
            self.connect(&new_term, &p1);
            term = p2;
            self.skip_whitespace();
        }
        self.in_stream.next();
        Ok(term)
    }
    fn parse_sup(&mut self, binder: bool, label: Label) -> Result<WireId> {
        self.in_stream.next().unwrap();
        let mut term = self.parse_term(binder)?;
        self.skip_whitespace();
        while self.in_stream.peek() != Some(&'}') {
            let new_term = self.parse_term(binder)?;
            let (p0, p1, p2) = self.add_node(true, true, false, label);
            self.connect(&term, &p1);
            self.connect(&new_term, &p2);
            term = p0;
            self.skip_whitespace();
        }
        self.in_stream.next();
        Ok(term)
    }
    fn label_or_generate(&mut self, q: Option<String>) -> Label {
        match q {
            None => {
                self.label_index += 1;
                self.label_index - 1
            }
            Some(label) => {
                match self.label_map.try_insert(label, self.label_index) {
                    Ok(_) => {
                        self.label_index += 1;
                        self.label_index - 1
                    },
                    Err(e) => e.value,
                }
            }
        }
    }
    fn new_dangling_wire(&mut self) -> WireId {
        self.dest_index += 1;
        WireId(self.dest_index - 1)
    }
    fn parse_skip_text(&mut self, text: &str){
        let mut curr_text = text;
        self.skip_whitespace();
        while curr_text.len() > 0 {
            if self.in_stream.next() == curr_text.chars().nth(0) {
                let char_idx = curr_text.char_indices().skip(1).map(|x| x.0).next().unwrap_or(curr_text.len());
                curr_text = &curr_text[char_idx..]
            } else {
                panic!("Expected {:?}", text);
            }
        }
    }
    fn parse_var(&mut self, name: String, binder: bool) -> Result<WireId> {
        let data = self.vars.entry(name);
        let data = data.or_default().clone();
        let mut data = (*data).borrow_mut();
        if binder {
            data.binder_count += 1;
            if let Some(_binder) = &mut data.to_binder {
                // Add dup node
                let (_p0, _p1, p2) = self.add_node(true, true, false, 1);
                Ok(p2)
            } else {
                let w = self.new_dangling_wire();
                data.to_binder = Some(w.clone());
                Ok(w)
            }
        } else {
            data.user_count += 1;
            if let Some(_user) = &mut data.to_user {
                // Add dup node
                let (_p0, _p1, p2) = self.add_node(true, true, false, 3);
                Ok(p2)
            } else {
                let w = self.new_dangling_wire();
                data.to_user = Some(w.clone());
                Ok(w)
            }
        }
    }
    fn connect(&mut self, a: &WireId, b: &WireId) {
        println!("{:?}", self.aliases);
        println!("{:?}", b);
        if let Some(b) = self.aliases.get(b) {
            self.connect(a, &b.clone())
        } else {
            for (k, v) in &mut self.aliases.iter_mut() {
                if *v == a.clone() {
                    *v = b.clone();
                }
            }
            let mut k = a.clone();
            while let Some(old) = self.aliases.insert(k, b.clone()) {
                k = old;
            }
        }
    }
    fn skip_whitespace(&mut self) {
        while let Some(x) = self.in_stream.peek() && x.is_ascii_whitespace() {
            self.in_stream.next();
        }
    }
    fn parse_term(&mut self, binder: bool) -> Result<WireId> {
        let curr_label = None;
        self.skip_whitespace();
        match self.in_stream.peek() {
            None => return Err("Expected term".into()),
            Some(x) => match x {
                'λ' | '@' => {
                    let label = 0;
                    self.parse_lam(binder, label)
                },
                '(' => {
                    let label = 0;
                    self.parse_app(binder, label)
                },
                '{' => {
                    let label = self.label_or_generate(curr_label);
                    self.parse_sup(binder, label)
                },
                _ => {
                    let name = self.parse_name()?;
                    if name == "let" {
                        let bind = self.parse_term(!binder)?;
                        self.parse_skip_text("=");
                        let value = self.parse_term(binder)?;
                        let body = self.parse_term(binder)?;
                        // let bind = value; body
                        // ---------------------
                        // ((λbind body) value)
                        // ---------------------
                        // bind ~ value; body
                        self.connect(&bind, &value);
                        println!("{:?}", bind);
                        println!("{:?}", value);
                        println!("{:?}", body);
                        Ok(body)
                    } else if name == "ask" {
                        let bind = self.parse_term(!binder)?;
                        self.parse_skip_text("=");
                        let value = self.parse_term(binder)?;
                        let body = self.parse_term(binder)?;
                        // ask bind = value; body
                        // ---------------------
                        // (value (λbind body))
                        let (l0, l1, l2) = self.add_node(true, true, false, 0);
                        self.connect(&l1, &bind);
                        self.connect(&l2, &body);

                        let (a0, a1, a2) = self.add_node(true, true, false, 0);
                        self.connect(&value, &a0);
                        self.connect(&l0, &a1);
                        Ok(a2)
                    } else {
                        self.parse_var(name, binder)
                    }
                },
            }
        }
    }
    fn close(&mut self, root: WireId) {
        for (_s, v) in self.vars.clone().iter_mut() {
            let v = v.borrow_mut();
            if let Some(mut binder) = v.to_binder.clone() {
                if let Some(mut user) = v.to_user.clone() {
                    self.connect(&mut binder, &mut user);
                }
            }
        }
        let mut var_addr: BTreeMap<WireId, Vec<Addr>> = BTreeMap::new();
        for (idx, node) in self.node_data.iter_mut().enumerate() {
            let idx = idx as u64;
            var_addr.entry(self.aliases.get(&node.p0).unwrap_or(&node.p0).clone()).or_default().push(Addr { node: idx, port: 0});
            var_addr.entry(self.aliases.get(&node.p1).unwrap_or(&node.p1).clone()).or_default().push(Addr { node: idx, port: 1});
            var_addr.entry(self.aliases.get(&node.p2).unwrap_or(&node.p2).clone()).or_default().push(Addr { node: idx, port: 2});
        }

        let mut mem: Vec<(Port, Port)> = vec![];
        let mut root_addr = None;
        let mut redex: Vec<(Port, Port)> = vec![];
        for _node in &self.node_data {
            mem.push((Port::default(), Port::default()))
        }
        let mut connect = |v0: &Addr, v1: &Addr, mem: &mut Vec<(Port, Port)>, nd: &[NodeData]| {
            // Connect them
            if v0.port == 0 && v1.port == 0 {
                let n1 = &nd[v0.node as usize];
                let n2 = &nd[v1.node as usize];
                redex.push((
                    Port {
                        is_primary: n1.primary,
                        is_vanilla: n1.vanilla,
                        is_lazy: n1.lazy,
                        target: v0.clone(),
                        label: n1.label,
                    },
                    Port {
                        is_primary: n2.primary,
                        is_vanilla: n2.vanilla,
                        is_lazy: n2.lazy,
                        target: v1.clone(),
                        label: n2.label,
                    },
                ))
            } else if v0.port == 0 {
                let q = v1.get_tuple_2(&mut mem[v1.node as usize]);
                let node = &nd[v0.node as usize];
                q.is_primary = node.primary;
                q.is_vanilla = node.vanilla;
                q.is_lazy = node.lazy;
                q.label = node.label;
                q.target = v0.clone();
            } else if v1.port == 0 {
                // Aux-aux connection
                let q = v0.get_tuple_2(&mut mem[v0.node as usize]);
                let node = &nd[v1.node as usize];
                q.is_primary = node.primary;
                q.is_vanilla = node.vanilla;
                q.is_lazy = node.lazy;
                q.label = node.label;
                q.target = v1.clone();
            } else {
                // Aux-aux connection
                let q = v0.get_tuple_2(&mut mem[v0.node as usize]);
                q.is_primary = false;
                q.is_vanilla = true;
                q.is_lazy = false;
                q.label = 0;
                q.target = v1.clone();
                let q = v1.get_tuple_2(&mut mem[v1.node as usize]);
                q.is_primary = false;
                q.is_vanilla = true;
                q.is_lazy = false;
                q.label = 0;
                q.target = v0.clone();
            }
        };
        println!("{:#?}", self.aliases);
        for (k, v) in var_addr {
            if v.len() == 1 {
                // Is this root?
                if k == root {
                    root_addr = Some(v[0].clone());
                    continue;
                }
                // Otherwise, erase
                let p1 = Port {
                    target: Addr { node: mem.len() as u64, port: 2 },
                    is_vanilla: true,
                    ..Default::default()
                };
                let p2 = Port {
                    target: Addr { node: mem.len() as u64, port: 1 },
                    is_vanilla: true,
                    ..Default::default()
                };
                self.node_data.push(NodeData {
                    vanilla: true,
                    lazy: false,
                    primary: true,
                    label: 0xFF,
                    p0: WireId(u64::MAX),
                    p1: WireId(u64::MAX),
                    p2: WireId(u64::MAX),
                });
                mem.push((p1, p2));
                connect(&v[0], &Addr {
                    node: mem.len() as u64 - 1,
                    port: 0,
                }, &mut mem, &self.node_data)
            } else if v.len() == 2 {
                connect(&v[0], &v[1], &mut mem, &self.node_data)
            } else if v.len() > 2 {
                todo!();
            }
        }
        let redex_start = mem.len() as u64;
        let redex_amount = redex.len() as u64;

        // Create root port
        let root_addr = root_addr.unwrap();
        
        let mut root_port = if root_addr.port == 0 {
            let node = &self.node_data[root_addr.node as usize];
            Port {
                is_primary: node.primary,
                is_lazy: node.lazy,
                is_vanilla: node.vanilla,
                target: root_addr.clone(),
                label: node.label,
            }
        } else {
            Port {
                is_primary: false,
                is_vanilla: true,
                is_lazy: false,
                target: root_addr.clone(),
                label: 0,
            }
        };
        let redex_num: u64 = if redex_amount == 0 {
            0
        } else {
            redex_start * 16 + redex_amount * 16 * 3 - 8
        };


        let root_port_num = root_port.to_bits() + 16;
        self.out_stream.write(
            &root_port_num.to_le_bytes(),
        ).unwrap();
        self.out_stream.write(
            &redex_num.to_le_bytes(),
        ).unwrap();

        // println!("ROOT {}", root_addr.unwrap());
        for (idx, (p1, p2)) in mem.iter().enumerate() {
            // println!("{}", idx);
            // println!(" .1: {}", p1);
            // println!(" .2: {}", p2);
            let mut p1 = p1.to_bits().wrapping_sub(idx as u64 * 16);
            let mut p2 = p2.to_bits().wrapping_sub(idx as u64 * 16 + 8);
            if idx as u64 == root_addr.node {
                if root_addr.port == 1 {
                    p1 = p1.wrapping_sub(14);
                }
                if root_addr.port == 2 {
                    p2 = p2.wrapping_sub(14);
                }
            }
            self.out_stream.write(
                &p1.to_le_bytes(),
            ).unwrap();
            self.out_stream.write(
                &p2.to_le_bytes(),
            ).unwrap();
        }
        for (idx, (p1, p2)) in redex.into_iter().enumerate() {
            let provenance = 0u64;
            let idx = idx as u64;
            let p1 = p1.to_bits().wrapping_sub((idx * 3 + redex_start) * 16);
            let p2 = p2.to_bits().wrapping_sub((idx * 3 + redex_start + 1) * 16);
            let provenance = provenance.wrapping_sub((idx * 3 + redex_start + 2) * 16);
            self.out_stream.write(
                &p1.to_le_bytes(),
            ).unwrap();
            self.out_stream.write(
                &8u64.to_le_bytes(),
            ).unwrap();
            self.out_stream.write(
                &p2.to_le_bytes(),
            ).unwrap();
            self.out_stream.write(
                &8u64.to_le_bytes(),
            ).unwrap();
            self.out_stream.write(
                &provenance.to_le_bytes(),
            ).unwrap();
            if redex_amount == idx + 1 {
                self.out_stream.write(
                    &(8i64-redex_amount as i64*16*3).to_le_bytes(),
                ).unwrap();
            } else {
                self.out_stream.write(
                    &8u64.to_le_bytes(),
                ).unwrap();
            }
        }
        self.out_stream.flush().unwrap();
    }
}

pub fn main() -> Result<()> {
    use std::fs::File;
    let mut args = std::env::args();
    args.next();
    let mut in_file = args.next().map(|x| Box::new(File::open(x).unwrap()) as Box<dyn Read>).unwrap_or(Box::new(std::io::stdin().lock()));
    let mut out_file = args.next().map(|x| Box::new(File::create(x).unwrap()) as Box<dyn std::io::Write>).unwrap_or(Box::new(std::io::stdout().lock()));

    let mut v = Vec::new();
    println!("{:?}", in_file.read_to_end(&mut v));
    println!("{:?}", v);
    let mut s = std::str::from_utf8(&v).unwrap().to_string();
    let mut state = State {
        out_stream: out_file,
        in_stream: s.chars().peekable(),
        vars: BTreeMap::new(),
        label_index: 0,
        node_index: 0,
        aliases: BTreeMap::new(),
        node_data: vec![],
        label_map: BTreeMap::new(),
        dest_index: 0,
    };
    let root = state.parse_term(false)?;
    state.close(root);

    Ok(())
}
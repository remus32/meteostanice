$fn=40;

r_sroubku=3/2;
r_tyce=1.1 * 10/2;

krabka_a=100;
krabka_b=60;
krabka_h=35;
krabka_h_zakladny=2;

standoff_h=10;
prepazka_h=krabka_h;

body_a=47;
body_b=24;
body=[
  [body_a/2, body_b/2, 2, 0.4,  180],
  [body_a/2, -body_b/2, 0, 0.4,  0],
  [-body_a/2, body_b/2, 0.9, 0.5,  270],
  [-body_a/2, -body_b/2, 0.5, 0.9,  0]
];

module standoff(r, h, vyrez_a, vyrez_b) {
  vyrez_h=5;
  difference() {
    cylinder(r=r, h=h, center=true);
    if (vyrez_a > 0) {
      translate([1e4+r_sroubku+vyrez_a, 0, h/2])
        cube(size=[2e4, 2e4, 2*vyrez_h], center=true);
    }
    if (vyrez_b > 0) {
      translate([0, 1e4+r_sroubku+vyrez_b, h/2])
        cube(size=[2e4, 2e4, 2*vyrez_h], center=true);
    }
  }
}
module sroubovy_sloupek() {
  a=6;

  translate([-a/2, -a/2, 0]) {
    linear_extrude(krabka_h)
      polygon([
        [0, 0],
        [0, a/2],
        [a/2, 0]
      ]);
    translate([a/2, a/2, 0])
      cylinder(d=a, h=krabka_h, center=false, $fn=40);
  }
}

module krabka() {
  // 4 standoffy pro modul
  translate([krabka_a/2-body_a/2-6, 0, standoff_h/2])
    for(i = body) {
      translate([i[0], i[1], 0]) rotate([0, 0, i[4]]) difference() {
        standoff(5, standoff_h, i[2], i[3]);
      }
    }

  // prepazka pred BMEckem
  translate([-30, -krabka_b/2, 0]) difference() {
    cube([krabka_h_zakladny, krabka_b, prepazka_h]);

    // výřez na kablíky
    translate([-krabka_h_zakladny, krabka_b/2-9/2, prepazka_h-7])
      cube([3*krabka_h_zakladny, 9, 1e4]);
  }

  // steny
  translate([0, 0, krabka_h/2]) difference() {
    cube(size=[krabka_a, krabka_b, krabka_h], center=true);
    translate([0, 0, krabka_h_zakladny])
      cube(size=[krabka_a-2*krabka_h_zakladny, krabka_b-2*krabka_h_zakladny, krabka_h], center=true);
    for(i=[0:1]) {
      range = i % 2 == 0 ? [-1:(2/4):1] : [-1:(2/3):1];
      for(j=range) {
        translate([-5-krabka_a/2, j*(0.6*krabka_b/2), -10 + i*10])
          rotate([0, 90, 0]) linear_extrude(10)
            circle(r=2);
      }
    }
  }

  translate([krabka_a/2 - krabka_h_zakladny - 3, krabka_b/2 - krabka_h_zakladny - 3, 0])
    rotate([0, 0, 180])
      sroubovy_sloupek();
  translate([krabka_a/2 - krabka_h_zakladny - 3, -krabka_b/2 + krabka_h_zakladny + 3, 0])
    rotate([0, 0, 90])
      sroubovy_sloupek();
  translate([-krabka_a/2 + krabka_h_zakladny + 3, krabka_b/2 - krabka_h_zakladny - 3, 0])
    rotate([0, 0, 270])
      sroubovy_sloupek();
  translate([-krabka_a/2 + krabka_h_zakladny + 3, -krabka_b/2 + krabka_h_zakladny + 3, 0])
    rotate([0, 0, 0])
      sroubovy_sloupek();
}

module diry() {
  x = krabka_a/2 - krabka_h_zakladny - 3;
  y = krabka_b/2 - krabka_h_zakladny - 3;

  for(i=[0:3]) {
    translate([
      i < 2 ? x : -x,
      i % 2 ? y : -y,
      0
    ])
      cylinder(r=r_sroubku, h=1e3, center=true, $fn=40);
  }
}

module krabka_s_dirama() {
  difference() {
    krabka();
    diry();

    // diry pro standoffy
    translate([krabka_a/2-body_a/2-6, 0, standoff_h/2])
      for(i = body) {
        translate([i[0], i[1], 0]) rotate([0, 0, i[4]])
          cylinder(r=1.5, h=30, center=true, $fn=40);
      }
  }
}

module viko_drzak(h) {
  x10=10;
  x11=6;
  y1=14;
  rotate([90, 0, 90]) linear_extrude(h)
    polygon([
      [-x10, 0],
      [x10, 0],
      [x11, y1],
      [-x11, y1]
    ]);
}
module viko() {
  h=5;

  translate([0, 0, -h/2]) rotate([180, 0, 0])
    difference() {
      cube([krabka_a + krabka_h_zakladny, krabka_b + krabka_h_zakladny, h], center=true);
      translate([0, 0, krabka_h_zakladny])
        cube([krabka_a + 0.5, krabka_b + 0.5, h], center=true);
    }


  difference() {
    union() {
      translate([krabka_a/2 - 25, 0, 0])
        viko_drzak(25);
    }
    translate([20, 0, 6]) rotate([0, 90, 0])
      cylinder(r=r_tyce, h=50, center=true);
  }
}
module viko_s_dirama() {
  difference() {
    viko();
    diry();

    for(i=[-1:2:1])
      translate([-40, 10*i, 0])
        cube([5, 2, 10], center=true);
  }
}

translate([0, -80, 5])
  viko_s_dirama();
krabka_s_dirama();

ombo_r = 88;

ombo_trychtyr_r1 = ombo_r;
ombo_trychtyr_r2 = 80;
ombo_trychtyr_r3 = 3;
ombo_trychtyr_h = 60;

ombo_clunek_g = 2;  // tloustka sten
ombo_clunek_d1 = 40; // delka jedne strany clunku
ombo_clunek_d2 = ombo_clunek_g; // sirka prostredku
ombo_clunek_h = 19; // vyska clunku (vcetne vysky podlahy)
ombo_clunek_w = 20; // sirka clunku
ombo_clunek_r_diry = 1;

ombo_clunek_d = ombo_clunek_d1 * 2 + ombo_clunek_d2;

ombo_spodek_r_vnejsi = ombo_r;
ombo_spodek_r_vnitrni = 80;
ombo_spodek_h_podlahy = 3;
ombo_spodek_h_clunku = 13; // vyska clunku nad podlahou

module ombo_trychtyr() {
  difference() {
    cylinder(r=ombo_trychtyr_r1, h=ombo_trychtyr_h);
    translate([0, 0, -0.01])
    cylinder(r2=ombo_trychtyr_r2, r1=ombo_trychtyr_r3, h=ombo_trychtyr_h + 0.02);
  }
}
translate([0, 0, 40])
ombo_trychtyr();

// https://cs.wikipedia.org/wiki/Sr%C3%A1%C5%BEkom%C4%9Br#/media/Soubor:Interior_tipping_bucket.JPG
module ombo_clunek() {
  module stena() {
    translate([0, ombo_clunek_g/2, 0]) // kvuli rotaci to neni vycentrovany
      rotate([90, 0, 0]) // exrude to polozi do xy planu, ja to chci nastojato
        linear_extrude(ombo_clunek_g)
          polygon([
            [ombo_clunek_d2/2, ombo_clunek_g/2],
            [ombo_clunek_d2/2, ombo_clunek_h],
            [ombo_clunek_d1 + ombo_clunek_d2/2, ombo_clunek_g/2]
          ]);
  }
  module steny() {
    translate([0, ombo_clunek_g/2-ombo_clunek_w/2, 0])
      stena();
    translate([0, -ombo_clunek_g/2+ombo_clunek_w/2, 0])
      stena();
  }

  translate([0, 0, -ombo_clunek_g/2])
    difference() {
      union() {
        translate([0, 0, ombo_clunek_g/2])
          rotate([0, 90, 90])
            cylinder(r=ombo_clunek_g, h=ombo_clunek_w, center=true);

        cube([ombo_clunek_d, ombo_clunek_w, ombo_clunek_g], center=true);
        translate([0, 0, ombo_clunek_h/2])
          cube([ombo_clunek_d2, ombo_clunek_w, ombo_clunek_h], center=true);

        steny();
        rotate([0, 0, 180])
          steny();
      }
      translate([0, 0, ombo_clunek_g/2])
        rotate([0, 90, 90])
          cylinder(r=ombo_clunek_r_diry, h=2*ombo_clunek_w, center=true);
    }
}
// ombo_clunek();

module ombo_dira() {
  pocet_car = 10;
  difference() {
    cylinder(r=20, h=ombo_spodek_h_podlahy);
    for (i=[0:pocet_car-1]) {
      translate([0, -20+(i/pocet_car*40), 0]) {
        cube([41, 1, 2], center=true);
      }
    }
  }
}

module ombo_spodek() {
  translate([0, 0, -ombo_spodek_h_podlahy])
    difference() {
      cylinder(r=ombo_spodek_r_vnejsi, h=ombo_spodek_h_podlahy);
      translate([50, 0, 0])
        ombo_dira();
      translate([-50, 0, 0])
        ombo_dira();
    }
  translate([0, 0, ombo_spodek_h_clunku])
    rotate([0, 18, 0])
      ombo_clunek();

  // translate([-22, 0, 6])
  //   cube([5, 1.5*ombo_clunek_w, 10], center=true);
  // translate([22, 0, 6])
  //   cube([5, 1.5*ombo_clunek_w, 10], center=true);

  module drzak() {
    a = 20;
    g = 4;
    translate([0, g/2, 0])
    rotate([90, 0, 0])
    linear_extrude(g)
      polygon([
        [-a, 0],
        [a, 0],
        [0, ombo_spodek_h_clunku * 4/3]
      ]);
  }

  difference() {
    union() {
      translate([0, 3/2+ombo_clunek_w/2+1, 0])
        drzak();
      translate([0, -(3/2+ombo_clunek_w/2+1), 0])
        drzak();
    }
    translate([0, 0, ombo_spodek_h_clunku])
      rotate([0, 90, 90])
        cylinder(r=ombo_clunek_r_diry, h=2*ombo_clunek_w, center=true);
  }

}
ombo_spodek();

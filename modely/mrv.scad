/* r_lopatky_vnitrni = 27;
r_lopatky = 30;
r_noha = 3;
h_noha = 80;
n_lopatek = 3;

r_hlava = 15;
h_hlava = 20;

r_spojka = 5;
h_spojka = 80;

r_rotor = 4;
magnet = 4;
h_spicka = 10;
h_rotor_1 = 5;
h_rotor_2 = 60;
h_rotor_3 = 60;
h_rotor = h_rotor_1 + magnet + h_rotor_2 + h_rotor_3;

h_lozisko = 5;
r_lozisko_vnitrni = 4;
r_lozisko_vnejsi = 13;
h_lozisko_okraj = 2;
r_lozisko_okraj = r_lozisko_vnitrni + 1;

r_stator_vnejsi = 20;
r_stator_vnitrni = r_lozisko_vnejsi;
h_stator_podlaha = 5;
h_stator_mezera = 5;
h_rotor_offset = h_stator_podlaha + h_stator_mezera;
h_stator = h_stator_podlaha + h_stator_mezera + h_rotor_1 + magnet + h_rotor_2; */

include <params.scad>;

module mrv_miska() {
  difference() {
    sphere(mrv_r_misky_vnejsi);
    sphere(mrv_r_misky_vnitrni);
    translate([2*mrv_r_misky_vnejsi, 0, 0])
      cube(4*mrv_r_misky_vnejsi, center=true);
  }
}

mrv_h_ruky = 2;
mrv_h_ruky_2 = 10;

module mrv_ruka_drzadlo() {
  /* linear_extrude(h=mrv_h_ruky)
    polygon([
      [0, 0],
      [mrv_d_ruky, ]
    ]) */
  cube([mrv_h_ruky, mrv_d_ruky, mrv_h_ruky_2], center=true);
}

module mrv_ruka() {
  translate([0, -mrv_d_ruky, 0])
    union() {
      translate([mrv_h_ruky / 2, 0, 0])
        mrv_miska();

      difference() {
        translate([0, mrv_d_ruky / 2, 0])
          mrv_ruka_drzadlo();
        sphere(mrv_r_misky_vnitrni);
      }
    }
}

module mrv_hlava() {
  for (i = [0:(mrv_n_rukou - 1)]) {
    rotate([0, 0, i * 360 / mrv_n_rukou]) mrv_ruka();
  }
  cylinder(r=mrv_r_stredu, h=mrv_h_stredu, center=true);
}

module mrv_noha() {
  /* cylinder(r=mrv_r_nohy, h=mrv_h_nohy_vevnitr + mrv_h_nohy_venku + mrv_h_stredu); */
  /* cylinder(r=mrv_r_zarazedla, h=mrv_h_zarazedla); */
  cylinder(h=mrv_h_nohy_venku_2, r1=mrv_r_loziska_vnitrni, r2=mrv_r_nohy_venku);
  translate([0, 0, mrv_h_nohy_venku_2 - 0.0001])
    cylinder(h=mrv_h_nohy_venku_1, r=mrv_r_nohy_venku);
}

module mrv() {
  translate([0, 0, mrv_h_nohy_venku_2 + mrv_h_nohy_venku_1 + mrv_h_stredu / 2])
    mrv_hlava();
  mrv_noha();
}

/* module rotor() {
  translate([0, 0, h_rotor]) hlava();
  translate([0, 0, h_rotor - h_rotor_3]) cylinder(r=r_lozisko_okraj, h=h_lozisko_okraj);
  difference() {
    cylinder(r=r_rotor, h=h_rotor);
    translate([0, 0, h_rotor_1 + (magnet / 2)]) cube([magnet, 1000, magnet], center=true);
  }
}

module stator_spodek() {
  difference() {
    cylinder(h=h_stator, r=r_stator_vnejsi);
    translate([0, 0, h_stator_podlaha]) cylinder(h=h_stator, r=r_stator_vnitrni);
  }
}

module stator_horejsek() {
  difference() {
    cylinder(h=h_stator, r=r_stator_vnejsi);
    translate([0, 0, h_stator_podlaha]) cylinder(h=h_stator, r=r_stator_vnitrni);
  }
}

module anemometr() {
  translate([0, 0, h_rotor_offset]) rotor();
  stator_horejsek();
  stator_spodek();
}

anemometr(); */

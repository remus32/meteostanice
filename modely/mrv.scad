include <params.scad>;

module mrv_miska() {
  difference() {
    sphere(mrv_r_misky_vnejsi);
    sphere(mrv_r_misky_vnitrni);
    translate([0, -2*mrv_r_misky_vnejsi, 0])
      cube(4*mrv_r_misky_vnejsi, center=true);
  }
}

mrv_h_ruky = 2;
mrv_h_ruky_2 = 10;

module mrv_ruka_drzadlo() {
  rotate([90, 0, 0]) translate([0, mrv_zamek_width, 0])
    linear_extrude(mrv_h_ruky)
      polygon(points=[
          [0, 0],
          [0, mrv_zamek_width],
          [mrv_d_ruky, 3],
          [mrv_d_ruky, 0]
      ]);
}

module mrv_ruka() {
  /* translate([0, -mrv_d_ruky, 0]) */
    union() {
      /* translate([mrv_h_ruky / 2, 0, 0]) */
        mrv_miska();

      difference() {
        translate([0, 0, 0])
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

mrv_r_loziska_vnitrni = 7/2;
mrv_r_loziska_vnejsi = 19/2;
mrv_h_loziska = 10;
mrv_magnet = 4;

mrv_r_misky_vnitrni = 38;
mrv_r_misky_vnejsi = 3 + mrv_r_misky_vnitrni;
mrv_zplosteni_misky = 1.1;

mrv_tloustka_drzaku = 2.5;
mrv_vyska_drzaku = 12;
mrv_tloustka_zamku_vnitni = 2;
mrv_tloustka_zamku_vnejsi = 15;

mrv_r_stredu = 20;
mrv_h_stredu = 10;
mrv_d_ruky = 120;
mrv_h_ruky = 5;
mrv_n_rukou = 3;
mrv_zamek_width = 8;

use <MCAD/regular_shapes.scad>;
use <MCAD/bearing.scad>;

module mrv_miska() {
  scale([1, 1 / mrv_zplosteni_misky, 1]) difference() {
    sphere(mrv_r_misky_vnejsi);
    sphere(mrv_r_misky_vnitrni);
    translate([0, -2*mrv_r_misky_vnejsi, 0])
      cube(4*mrv_r_misky_vnejsi, center=true);
  }
}

module mrv_ruka_zamek() {
  x0 = 0;
  x1 = 15;
  x2 = 18;
  x3 = 21;

  y0 = 0;
  y1 = mrv_tloustka_zamku_vnejsi / 2 - mrv_tloustka_zamku_vnitni;
  y2 = mrv_tloustka_zamku_vnejsi / 2 + mrv_tloustka_zamku_vnitni;
  y3 = mrv_tloustka_zamku_vnejsi;

  linear_extrude(mrv_vyska_drzaku)
    polygon([
      [x0, y0],
      [x1, y3],

      [x1, y2],
      [x2, y2],

      [x2, y3],
      [x3, y3],
      [x3, y0],
      [x2, y0],

      [x2, y1],
      [x1, y1],

      [x1, y0]
    ]);
}
module mrv_ruka_drzadlo() {
  cube([mrv_d_ruky, mrv_tloustka_drzaku, mrv_vyska_drzaku]);
  translate([mrv_d_ruky - 15, 0, 0])
    mrv_ruka_zamek();
}

module mrv_ruka() {
  translate([mrv_d_ruky, -mrv_tloustka_zamku_vnejsi/2, 0]) rotate([0, 180, 0])
    union() {
      mrv_miska();

      difference() {
        translate([0, 0, -mrv_vyska_drzaku/2])
          mrv_ruka_drzadlo();
        sphere(mrv_r_misky_vnitrni);
      }
    }
}

module mrv_hlava() {
  difference() {
    cylinder(r=20, h=15, center=true);
    for (i = [0:(mrv_n_rukou - 1)]) {
      scale([1.02, 1.02, 1.08])
        rotate([0, 0, i * 360 / mrv_n_rukou])
          translate([17.6, 0, 1])
            mrv_ruka();
    }
  }
}

module mrv_tyc() {
  delka_kuzelu = 6;
  delka_tyce = 30;
  h_podlahy = 3;

  translate([0, 0, -delka_kuzelu])
    cylinder(r1=mrv_r_loziska_vnitrni, r2=20, h=delka_kuzelu);
  translate([0, 0, -(delka_kuzelu+delka_tyce)])
    difference() {
      cylinder(r=mrv_r_loziska_vnitrni, h=delka_tyce);
      translate([0, 0, 5])
        cube([1000, mrv_magnet, mrv_magnet], center=true);
    }
}

module mrv() {
  translate([0, 0, 15/2]) {
    mrv_hlava();
    for (i = [0:(mrv_n_rukou - 1)]) {
      rotate([0, 0, i * 360 / mrv_n_rukou])
        translate([17.6, 0, 0])
          mrv_ruka();
    }
  }
  mrv_tyc();
}

mrv();

module prostredek() {
  p_stena = 8;
  p_r = mrv_r_loziska_vnejsi + p_stena;
  p_h = 50;
  h_spodku = p_h - mrv_h_loziska - p_stena;
  difference() {
    cylinder(r=p_r, h=p_h);
    translate([0, 0, p_h + 0.01 - mrv_h_loziska])
      cylinder(r=mrv_r_loziska_vnejsi + 0.1, h=mrv_h_loziska);
    translate([0, 0, -0.01])
      linear_extrude(h_spodku)
        octagon(mrv_r_loziska_vnejsi);
    //   cylinder(r=mrv_r_loziska_vnejsi + 0.1, h=h_spodku);
    cylinder(r=mrv_r_loziska_vnitrni+1, h=50);

  }

}

translate([0, 0, -12]) bearing(model=607);
translate([0, 0, -56]) prostredek();
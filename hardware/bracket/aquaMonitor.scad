
glassX = 11;
railZ = 6;
railY = 20;
railX = 70;
railWall = 2;
railSlotY = 6;
topZ = 10;
topX = 33;
topY = 44;
teeth = 3;
sphereD = 6;


rail();

module rail() {
  difference() {
    railBody();
    translate([railWall, railY - 2*railSlotY, 2*railWall-0.5]) {
      railTeeth();
    }
  }
}

module railBody() {
  difference() {
    union() {      
      cube([railX, railY, railZ]);
      for(i = [1:1:railX/(2*sphereD)]) {
        translate([i*2*sphereD, (railY - sphereD)/4, sphereD/2 - 1 ])
          sphere(d = sphereD , $fn=50);
        translate([i*2*sphereD, railY - (railY - sphereD)/4, sphereD/2 - 1 ])
          sphere(d = sphereD , $fn=50);
      }        
    }
    translate([railWall, railWall, railWall])
      cube([railX , railY - 2*railWall, railZ - 2*railWall]);
  }
}

module railTeeth() {
  teethZ = railZ - railWall;
  teethOffset = sqrt(pow(teeth, 2)/2);
  cube([railX , railSlotY, teethZ]);
  
  for(i = [1:1:railX/(teethOffset*2)]) {
    translate([teethOffset *2* i, -teethOffset, 0])
      rotate(45, [0, 0, 1]) {
        cube([teeth , teeth, teethZ]);  
    }
  }
  
}
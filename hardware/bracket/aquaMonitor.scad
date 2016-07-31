
glassX = 6;
railZ = 6;
railY = 20;
railX = 70;
railWall = 2;
railSlotY = 6;

teeth = 3;
teethZ = railZ - railWall;
teethOffset = sqrt(pow(teeth, 2)/2);
  
sphereD = 6;
topRecessX = 15;
levelWireTubeDiam = 5;
wireTubeDiam = 6;

topZ = 10;
topX = glassX + 4 + 2 * railZ + topRecessX;
topY = 44;

insertLength = 23.5;
insertBorder = 3;
insertWidth = railY - 2*railWall;
insertHeight = railZ - 2*railWall;
insertPawlWidth = 2.5;
 
armHeight = 24;
armWidth = insertBorder+1 ;


// Bracket body
//body();

// Insert
// insert();

// Demo
demo();




// Insert positionned within bracket, for demo, not printing.
module demo() {
  body();
  translate([topX - topRecessX - railZ + railWall,  
            insertWidth + (topY - railY)/2 + railWall,
            railX - insertLength -.5]) {
    rotate([180, -90, 0]) {
      insert();
    }
  }
}


module insert() { 
  extDiam = 11;
  intDiam = 6.2;
  
  difference() {
    cube([insertLength, insertWidth, insertHeight]);
    translate([insertBorder, insertBorder, -0.5]) {
      cube([insertLength - 2*insertBorder,
         insertWidth - 2*insertBorder, insertHeight + 1]);  
    }
  }
  // Pawl with tooth
  translate([0, (insertWidth - insertPawlWidth)/2, 0]) {
    pawlLength = insertLength - insertBorder - 1.2;
    cube([pawlLength, insertPawlWidth, 2*insertHeight]);
    translate([pawlLength - teethOffset, -teethOffset, 0]) {
      rotate(45, [0, 0, 1]) {
        cube([teeth , teeth, teethZ]);  
      }
    }
  }
  // Thicker border at Pawl attachement
  cube([armWidth, insertWidth, insertHeight]);
  
  // Vertical arm
  translate([0, (insertWidth - armWidth)/2, 0]) {
    cube([armWidth, armWidth, armHeight]);
  }
  translate([0, insertWidth/2, armHeight + extDiam - 1]) {
    rotate([0, 90, 0]) {
      difference() {
        union() {
          cylinder(r=extDiam, h=armWidth, $fn=50);
          translate([3-extDiam, -extDiam, 0]) {
            cube([extDiam - 3, extDiam*2, armWidth]);
          }
        }
        translate([0, 0, -0.5]) {
          cylinder(r=intDiam, h=armWidth+1, $fn=50);
        }
        translate([-intDiam*2, -intDiam, -0.5]) {
          cube([intDiam *2, intDiam*2, armWidth +1]);
        }        
        translate([-extDiam-3, -extDiam -0.5, -0.5]) {
          cube([extDiam - 3, extDiam*2 + 1, armWidth +1]);
        }        
      }
    }
  }
  
}

module body() { 
  difference() {
    cube([topX, topY, topZ]);
    translate([topX - topRecessX, (topY - railY)/2, -0.5])
      cube([topRecessX + 1, railY, topZ + 1]);
    
    // Hole for level sensor wires
    translate([-1, topY/2, topZ/2])
      rotate(90, [0, 1, 0])
        cylinder(d=levelWireTubeDiam, h=topX + 1, $fn=50);
    
    // Horizontal hole for light sensor wires
    translate([-2, (topY - railY)/4, topZ/2])
      rotate(90, [0, 1, 0])
        cylinder(d=wireTubeDiam, h=topX, $fn=50);
    // Vertical hole for light sensor
    translate([topX - wireTubeDiam + 1 , (topY - railY)/4, -topZ/2])
      cylinder(d=wireTubeDiam+1.5, h=topZ, $fn=50);    
    
    // Horizontal hole for temperature sensor wires
    translate([-2, topY - (topY - railY)/4, topZ/2])
      rotate(90, [0, 1, 0])
        cylinder(d=wireTubeDiam, h=topX, $fn=50);
    // Vertical hole for temperature sensor
    translate([topX - wireTubeDiam + 1 , topY - (topY - railY)/4, topZ/2])
      cylinder(d=wireTubeDiam+1, h=topZ, $fn=50);     
  }
  translate([topX - wireTubeDiam - 10, topY - (topY - railY)/4 - wireTubeDiam/2, topZ])
    temperatureSensorGuide();
  
  translate([topX - topRecessX - railZ, railY + (topY - railY)/2, topZ])
    rotate([0, -90, 180])
      rail();
  
  
  translate([0, (topY - railY)/2, topZ])
    rotate([0, 7, 0])
      press();
}

module temperatureSensorGuide() {
  translate([0, wireTubeDiam/2, 0])
    cylinder(d = wireTubeDiam, h = railX, $fn = 50);
  difference() {
    cube([7, wireTubeDiam, railX]);
    translate([9, wireTubeDiam/2, -0.5])
      cylinder(d = wireTubeDiam, h = railX + 1, $fn = 50);
  }
}

module press() {
  difference() {
    cube([railZ, railY, railX/2]);
    rotate([0, 5, 0]) {
      translate([-railZ, -0.5, 0])
        cube([railZ, railY+1, railX]);
    }
    translate([railZ+3, -0.5, railX/3])
      rotate([0, -25, 0]) {
        cube([railZ, railY+1, railX/2]);
    }    
  }

}

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
    translate([railWall, railWall-0.3, railWall - 0.3])
      cube([railX , railY - 2*railWall+0.6, railZ - 2*railWall + 0.6]);
  }
}

module railTeeth() {
  cube([railX , railSlotY, teethZ]); 
  for(i = [1:1:railX/(teethOffset*2)]) {
    translate([teethOffset *2* i, -teethOffset, 0])
      rotate(45, [0, 0, 1]) {
        cube([teeth , teeth, teethZ]);  
    }
  }
  
}
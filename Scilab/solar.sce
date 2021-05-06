[z]=read("/Users/john/Share/Repository/Chicken/Scilab/solar.log", -1, 1);


dataPoints = size(z,1);
dt = 1;
dx = 0;
x = z(1);

k = 0.01;
ka = .05 ;
kb = .05 ;


xArray = [];
dxArray = [];

for n = 1 : dataPoints,
  
  x = x + dx * dt;
  dx = dx;
  
  y = z(n) - x;
  
  x = x + ka * y;
  dx = dx + kb * y;
  
  xArray = [xArray x];
  dxArray = [dxArray dx];

end

// plot(xArray);
// plot(z);
//plot(diff(z));
plot(convol(z, z));
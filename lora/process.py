import sys
from datetime import datetime

print("lon,lat")
with open(0, 'rb') as stdin:
    for line in stdin:
        line = line.decode('utf-8','ignore')
        sline = line.split()
        #print(sline)
        if(len(sline)<2):
          continue
        try:
          lon = -float(sline[0])
          lat = float(sline[1])
        except Exception as e:
          continue
        if(lat == 999999999 or lon == 999999999):
          continue
        while(lat > 90):
          lat = lat/10
        while(lon < -180):
          lon = lon/10
        print(f'{lon}, {lat}')

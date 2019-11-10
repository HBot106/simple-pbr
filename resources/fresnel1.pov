camera {
  location  <0, 10, 30>
  up        <0,  1,  0>
  right     <1.33333, 0,  0>
  look_at   <0, 0, 0>
}


light_source {<0, 20, 30> color rgb <1.5, 1.5, 1.5>}

// lense 1
sphere { <-9, 8, 9>, 2
  pigment { color rgbf <1.0, 1.0, 1.0, 0.95> }
  finish { ambient 0.02 diffuse 0.4 specular 10.0 roughness 0.03 refraction 1.0 ior 1.05 }
}

// lense 2
sphere { <-6, 7, 0>, 2
  pigment { color rgbf <1.0, 1.0, 1.0, 0.95>}
  finish { ambient 0.02 diffuse 0.4 specular 10.0 roughness 0.03 refraction 1.0 ior 1.10 }
}

// lense 3
sphere { <-3, 6, 8>, 2
  pigment { color rgbf <1.0, 1.0, 1.0, 0.95>}
  finish { ambient 0.02 diffuse 0.4 specular 10.0 roughness 0.03 refraction 1.0 ior 1.15 }
}

// lense 4
sphere { <3, 5, -8>, 2
  pigment { color rgbf <1.0, 1.0, 1.0, 0.95> }
  finish { ambient 0.02 diffuse 0.4 specular 10.0 roughness 0.03 refraction 1.0 ior 1.20 }
}

// lense 5
sphere { <6, 4, 7>, 2
  pigment { color rgbf <1.0, 1.0, 1.0, 0.95>}
  finish { ambient 0.02 diffuse 0.4 specular 10.0 roughness 0.03 refraction 1.0 ior 1.25 }
}

// lense 6
sphere { <9, 3, -7>, 2
  pigment { color rgbf <1.0, 1.0, 1.0, 0.95>}
  finish { ambient 0.02 diffuse 0.4 specular 10.0 roughness 0.03 refraction 1.0 ior 1.30 }
}



// Mirror
sphere { <0, 5, -9>, 2
  pigment { color rgb <1.0, 1.0, 1.0> }
  finish {roughness 0.02 ior 1.6 reflection 1.0}
}


// Green
sphere { <-9, 4, 6>, 2
  pigment { color rgb <0.4, 1.0, 0.4> }
  finish {ambient 0.2 diffuse 0.6 specular 0.7 roughness 0.02 ior 1.6 reflection 0.6}
}

// Blue
sphere { <-6, 5, -6>, 2
  pigment { color rgb <0.4, 0.6, 1.0> }
  finish {ambient 0.2 diffuse 0.6 specular 0.7 roughness 0.02 ior 1.6 reflection 0.6}
}

// Purple
sphere { <-8, 6, -8>, 2
  pigment { color rgb <0.8, 0.4, 1.0> }
  finish {ambient 0.2 diffuse 0.6 specular 0.7 roughness 0.02 ior 1.6 reflection 0.6}
}

// Orange
sphere { <3, 7, -5>, 2
  pigment { color rgb <1.0, 0.65, 0.0> }
  finish {ambient 0.2 diffuse 0.6 specular 0.7 roughness 0.02 ior 1.6 reflection 0.6}
}

// Yellow
sphere { <6, 8, 4>, 2
  pigment { color rgb <1.0, 1.0, 0.4> }
  finish {ambient 0.2 diffuse 0.6 specular 0.7 roughness 0.02 ior 1.6 reflection 0.6}
}

// Red
sphere { <9, 9, -4>, 2
  pigment { color rgb <1.0, 0.3, 0.3> }
  finish {ambient 0.2 diffuse 0.6 specular 0.7 roughness 0.02 ior 1.6 reflection 0.6}
}

// Floor
plane {<0, 1, 0>, -5
  pigment { color rgb <0.62, 0.62, 0.62> }
  finish { ambient 0.4 diffuse 0.8 }
}

// Walls
plane {<-1, 0, 0>, -12
  pigment { color rgb <0.20, 0.60, 0.00> }
  finish { ambient 0.4 diffuse 0.8 }
}

plane {<1, 0, 0>, -12
  pigment { color rgb <0.62, 0.0, 0.34> }
  finish { ambient 0.4 diffuse 0.8 }
}

plane {<0, 0, 1>, -12
  pigment { color rgb <0.00, 0.50, 0.73> }
  finish { ambient 0.4 diffuse 0.8 }
}


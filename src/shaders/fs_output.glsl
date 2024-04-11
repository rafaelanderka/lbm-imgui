#version 330 core
// Visualises the velocity field and solutes

precision mediump float;
precision mediump sampler2D;

const vec3 backgroundCol1 = vec3(0.95);
const vec3 backgroundCol2 = vec3(0.9);
const vec3 velocityCol = vec3(1.); 
const vec4 wallCol = vec4(0., 0., 0., 1.);
const float checkerSize = 10.f;
const float doubleCheckerSize = checkerSize * 2.f;
const float indicatorOffset = 50.f;
const float indicatorArrowWidth = 0.13;
const float indicatorArrowScale = 0.7;
const float minIndicatorSpeed = 1e-7;

uniform sampler2D uNodeIds;
uniform sampler2D uFluidData[4];
uniform sampler2D uSolute0Data;
uniform sampler2D uSolute1Data;
uniform sampler2D uSolute2Data;
uniform vec3 uSolute0Col;
uniform vec3 uSolute1Col;
uniform vec3 uSolute2Col;
uniform vec2 uAspect;
uniform vec2 uCursorPos;
uniform float uAnimationPhase;
uniform float uToolSize;
uniform vec2 uViewportSize;
uniform vec2 uViewportScale;
uniform bool uDrawIndicatorLines;
uniform bool uDrawIndicatorArrows;
uniform bool uDrawCursor;

in vec2 UV; 

out vec4 outColor;

vec2 getIndicatorUV() {
  return UV - vec2(mod(UV.x, indicatorOffset), mod(UV.y, indicatorOffset)) + 0.5 * indicatorOffset;
}

float getToneMappedConcentration(sampler2D solute) {
  return sqrt(clamp(texture(solute, UV).x, 0., 1.));
}

float getToneMappedVelocity() {
  return 0.2 * length(texture(uFluidData[0], UV).xy);
}

float signedDistanceSegment(vec2 p, vec2 offset) {
  vec2 UVa = uAspect * (UV - p);
  offset *= uAspect;
  float h = clamp(dot(UVa, offset) / dot(offset, offset), 0., 1.);
  return length(UVa - offset * h);
}

float signedDistanceTriangle(vec2 p0, vec2 p1, vec2 p2) {
  vec2 e0 = uAspect * (p1 - p0);
  vec2 e1 = uAspect * (p2 - p1);
  vec2 e2 = uAspect * (p0 - p2);
  vec2 v0 = uAspect * (UV - p0);
  vec2 v1 = uAspect * (UV - p1);
  vec2 v2 = uAspect * (UV - p2);
  vec2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0., 1.);
  vec2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0., 1.);
  vec2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0., 1.);
  float s = sign(e0.x * e2.y - e0.y * e2.x);
  vec2 d = min(min(vec2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),
                   vec2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),
                   vec2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));
  return -sqrt(d.x) * sign(d.y);
}

void main(void) {
  // Shade fluid and solutes
  float c0 = getToneMappedConcentration(uSolute0Data);
  float c1 = getToneMappedConcentration(uSolute1Data);
  float c2 = getToneMappedConcentration(uSolute2Data);
  float v = getToneMappedVelocity();
  vec4 solute0 = vec4(c0 * uSolute0Col, c0);
  vec4 solute1 = vec4(c1 * uSolute1Col, c1);
  vec4 solute2 = vec4(c2 * uSolute2Col, c2);
  vec4 velocity = vec4(v * velocityCol, v);
  vec4 soluteBlend = vec4(1.) - ((vec4(1.) - solute0) * (vec4(1.) - solute1) * (vec4(1.) - solute2) * (vec4(1.) - velocity));

  // Fix premultiplied alpha fringing
  soluteBlend = vec4(soluteBlend.x / soluteBlend.w, soluteBlend.y / soluteBlend.w, soluteBlend.z / soluteBlend.w, soluteBlend.w);

  // Add background color
  vec2 pixelCoords = vec2(gl_FragCoord.x, gl_FragCoord.y) / uViewportScale;
  vec3 nodalBackgroundCol = (mod(pixelCoords.x, doubleCheckerSize) < checkerSize) != (mod(pixelCoords.y, doubleCheckerSize) < checkerSize) ? backgroundCol1 : backgroundCol2;
  soluteBlend = vec4(soluteBlend.xyz * soluteBlend.w + nodalBackgroundCol * (1 - soluteBlend.w), 1.);

  // Determine nearest indicator data
  // vec2 indicatorUV = getIndicatorUV();
  vec2 indicatorPixelLoc = pixelCoords - vec2(mod(pixelCoords.x, indicatorOffset), mod(pixelCoords.y, indicatorOffset)) + indicatorOffset / 2;
  vec2 indicatorUV = indicatorPixelLoc * uViewportScale / uViewportSize;
  vec2 indicatorVel = texture(uFluidData[0], indicatorUV).xy * uViewportScale * uAspect / uViewportSize * 128.f;
  float indicatorSpeed = dot(indicatorVel, indicatorVel);

  // Calculate indicator line shading
  float indicatorLineDistanceField = signedDistanceSegment(indicatorUV, indicatorVel);
  float indicatorLineThickness = max(uViewportScale.x / uViewportSize.x, uViewportScale.y / uViewportSize.y);
  bool shadeIndicator = uDrawIndicatorLines && (indicatorSpeed > minIndicatorSpeed) && (indicatorLineDistanceField < indicatorLineThickness);

  // Calculate indicator arrow shading
  vec2 indicatorArrowOffset = indicatorUV + 0.03 * indicatorVel;
  vec2 indicatorArrowP1 = indicatorArrowOffset + (indicatorArrowScale * indicatorVel) / uAspect;
  vec2 indicatorArrowP0 = indicatorArrowOffset + (indicatorArrowScale * vec2(indicatorVel.y * indicatorArrowWidth, -indicatorVel.x * indicatorArrowWidth)) / uAspect;
  vec2 indicatorArrowP2 = indicatorArrowOffset + (indicatorArrowScale * vec2(-indicatorVel.y * indicatorArrowWidth, indicatorVel.x * indicatorArrowWidth)) / uAspect;
  float indicatorArrowDistanceField = signedDistanceTriangle(indicatorArrowP0, indicatorArrowP1, indicatorArrowP2);
  shadeIndicator = shadeIndicator || (uDrawIndicatorArrows && (indicatorSpeed > minIndicatorSpeed) && (indicatorArrowDistanceField < 8e-4));

  // Determine whether node is adjacent to wall
  float offsetX = 2.f / uViewportSize.x * uViewportScale.x;
  float offsetY = 2.f / uViewportSize.y * uViewportScale.y;
  vec2 UV_t  = UV + vec2(      0.,  offsetY);
  vec2 UV_tr = UV + vec2( offsetX,  offsetY);
  vec2 UV_r  = UV + vec2( offsetX,       0.);
  vec2 UV_br = UV + vec2( offsetX, -offsetY);
  vec2 UV_b  = UV + vec2(      0., -offsetY);
  vec2 UV_bl = UV + vec2(-offsetX, -offsetY);
  vec2 UV_l  = UV + vec2(-offsetX,       0.);
  vec2 UV_tl = UV + vec2(-offsetX,  offsetY);
  bool hasAdjacentWall = (texture(uNodeIds, UV_t).x + texture(uNodeIds, UV_tr).x + texture(uNodeIds, UV_r).x + texture(uNodeIds, UV_br).x + texture(uNodeIds, UV_b).x + texture(uNodeIds, UV_bl).x + texture(uNodeIds, UV_l).x + texture(uNodeIds, UV_tl).x) > .99;

  // Determine node type
  float nodeId = texture(uNodeIds, UV).x;
  bool isFluid = nodeId < .7;
  bool isWall = nodeId > .1;
  float wallOutlineAlpha = (isFluid && hasAdjacentWall) ? 1. : 0.;
  float maxAspect = max(uAspect.x, uAspect.y);
  vec2 scaledUV = uAspect * UV;
  float wallStripeAlpha = isWall ? 5.f * clamp(pow(sin(0.4f * (pixelCoords.x + pixelCoords.y) + uAnimationPhase), 10.f), 0.f, 0.2f) : 0.f;
  float wallAlpha = wallOutlineAlpha + wallStripeAlpha;

  // Shade cursor
  float distanceFromCursor = length((uAspect * uCursorPos) - (uAspect * UV)) - uToolSize;
  bool shadeCursor = uDrawCursor && distanceFromCursor < 8e-4 && distanceFromCursor > -8e-4;

  vec4 fluidComposite = (1.f - wallAlpha) * soluteBlend + wallAlpha * wallCol;
  vec4 fluidCompositeInv = vec4(vec3(1.) - fluidComposite.xyz, 1.);
  outColor = (shadeIndicator || shadeCursor) ? fluidCompositeInv : fluidComposite;
}

import jwt from "jsonwebtoken";

const ACCESS_SECRET = process.env.JWT_ACCESS_SECRET;
const REFRESH_SECRET = process.env.JWT_REFRESH_SECRET;
const ACCESS_EXPIRES = process.env.JWT_ACCESS_EXPIRES || "15m";

export async function POST(req) {
  try {
    const body = await req.json();
    const { refreshToken } = body;
    // this part was sourced from generative ai, and I understood how it worked.
    if (!refreshToken) {
      return Response.json({ error: "Refresh token required" }, { status: 400 });
    }
    let decoded;
    try {
      decoded = jwt.verify(refreshToken, REFRESH_SECRET);
    } catch (error) {
      return Response.json({ error: "Invalid or expired refresh token" }, { status: 401 });
    }
    const payload = {
      userId: decoded.userId,
      username: decoded.username,
      role: decoded.role,
      expiresAt: Math.floor(Date.now() / 1000) + 60 * 15,
    };

    const newAccessToken = jwt.sign(payload, ACCESS_SECRET, { expiresIn: ACCESS_EXPIRES });

    return Response.json({ accessToken: newAccessToken }, { status: 200 });
  } catch (error) {
    console.error("Refresh Error:", error);
    return Response.json({ error: "Internal server error" }, { status: 500 });
  }
}

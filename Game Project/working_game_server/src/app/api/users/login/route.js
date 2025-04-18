import bcrypt from "bcrypt";
import jwt from "jsonwebtoken";
import { PrismaClient } from "@prisma/client";

const prisma = new PrismaClient();

// Load secrets from environment variables
const ACCESS_SECRET = process.env.JWT_ACCESS_SECRET;
const REFRESH_SECRET = process.env.JWT_REFRESH_SECRET;
const ACCESS_EXPIRES = process.env.JWT_ACCESS_EXPIRES || "15m";
const REFRESH_EXPIRES = process.env.JWT_REFRESH_EXPIRES || "7d";

export async function POST(req) {
  try {
    const body = await req.json();
    const { username, password } = body;

    if (!username || !password) {
      return Response.json({ error: "Username and password are required" }, { status: 400 });
    }

    // Check if user exists
    const user = await prisma.user.findUnique({ where: { username } });

    if (!user) {
      return Response.json({ error: "Invalid credentials" }, { status: 401 });
    }

    // Validate password
    const passwordMatch = await bcrypt.compare(password, user.password);
    if (!passwordMatch) {
      return Response.json({ error: "Invalid credentials" }, { status: 401 });
    }

    // Generate JWT payload
    const payload = {
      userId: user.id,
      username: user.username,
      role: user.role,
      expiresAt: Math.floor(Date.now() / 1000) + 60 * 15, // Expiry in seconds
    };

    // Generate Access Token (Short-lived)
    const accessToken = jwt.sign(payload, ACCESS_SECRET, { expiresIn: ACCESS_EXPIRES });

    // Generate Refresh Token (Long-lived)
    const refreshToken = jwt.sign(payload, REFRESH_SECRET, { expiresIn: REFRESH_EXPIRES });

    return Response.json({ accessToken, refreshToken }, { status: 200 });

  } catch (error) {
    console.error("Login Error:", error);
    return Response.json({ error: "Internal server error" }, { status: 500 });
  }
}

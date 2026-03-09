import bcrypt from "bcrypt";
import { PrismaClient } from "@prisma/client";

const prisma = new PrismaClient();

export async function POST(req) {
  try {
    const body = await req.json();
    const { username, password, role = "USER" } = body;

    if (!username || !password) {
      return Response.json({ error: "Username AND password are required" }, { status: 400 });
    }

    const validRoles = ["ADMIN", "USER"];
    if (!validRoles.includes(role)) {
      return Response.json({ error: "Invalid role" }, { status: 400 });
    }

    const existingUser = await prisma.user.findUnique({ where: { username } });

    if (existingUser) {
      return Response.json({ error: "Username already exists" }, { status: 400 });
    }

    const hashedPassword = await bcrypt.hash(password, 10);

    const user = await prisma.user.create({
      data: { username, password: hashedPassword, role },
    });

    return Response.json({
        message: "User registered successfully",
        user: {
          id: user.id,
          username: user.username,
          role: user.role,
          createdAt: user.createdAt,
        }
      }, { status: 200 });
      

  } catch (error) {
    return Response.json({ error: "Internal server error" }, { status: 500 });
  }
}

import { verifyToken } from "@/lib/auth";

export async function GET(req) {
  const user = verifyToken(req); // Verify token

  if (!user) {
    return Response.json({ error: "Unauthorized" }, { status: 401 });
  }
  return Response.json({ message: "Protected data accessed!", user }, { status: 200 });
}

import { verifyToken } from "@/lib/auth";

export async function GET(req) {
  const user = verifyToken(req);

  if (!user) {
    return Response.json({ error: "Unauthorized" }, { status: 401 });
  }

  if (user.role !== "ADMIN") {
    return Response.json({ error: "Forbidden: Administrators only" }, { status: 403 });
  }

  return Response.json({ message: "Admin-only data successfully accessed!", user }, { status: 200 });
}

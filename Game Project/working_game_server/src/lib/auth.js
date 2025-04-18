import jwt from "jsonwebtoken";

const ACCESS_SECRET = process.env.JWT_ACCESS_SECRET;

// Middleware
export function verifyToken(req, requiredRole = null) {
  try {
    const authHeader = req.headers.get("authorization");

    if (!authHeader || !authHeader.startsWith("Bearer ")) {
      return null; // No token
    }

    const token = authHeader.split(" ")[1]; // Extract the token
    const decoded = jwt.verify(token, ACCESS_SECRET);
    if (requiredRole && decoded.role !== requiredRole) {
      return null;
    }

    return decoded; // authentication is successful
  } catch (error) {
    console.error("Token verification failed:", error.message);
    return null;
  }
}

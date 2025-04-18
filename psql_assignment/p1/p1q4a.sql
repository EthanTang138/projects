-- Ethan Tang, tangeth4, 1009681471
-- a)
SELECT p.Type, SUM(f.Fee) AS revenue
FROM Passenger p
JOIN Fare f ON p.Type = f.Type
GROUP BY p.Type;

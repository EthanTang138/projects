-- Ethan Tang, tangeth4, 1009681471
-- c)
SELECT t.Date, COUNT(t.SIN) AS "trips taken"
FROM Take t
GROUP BY t.Date
ORDER BY COUNT(t.SIN) DESC
LIMIT 1;

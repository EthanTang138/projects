-- Ethan Tang, tangeth4, 1009681471
-- a)
SELECT p.Occupation, COUNT(*) AS occurrences
FROM Person p
JOIN Take t ON t.SIN = p.SIN
JOIN Ship sh ON t.ShipID = sh.ShipID
JOIN Schedule sc ON sh.RouteID = sc.RouteID
JOIN Stop st ON st.StopID = sc.StopID
JOIN Sites s ON st.SIName = s.SIName AND s.PhoneNumber = st.PhoneNumber 
WHERE s.category = 'Library'
  AND sc.Date IN ('2023-09-05', '2023-09-06')
GROUP BY p.Occupation;
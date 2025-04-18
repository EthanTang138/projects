-- Ethan Tang, tangeth4, 1009681471
-- b)
WITH OccupationVisits AS (
    SELECT p.Occupation, COUNT(*) AS occurrences, sc.Date
    FROM Person p
    JOIN Take t ON t.SIN = p.SIN
    JOIN Ship sh ON t.ShipID = sh.ShipID
    JOIN Schedule sc ON sh.RouteID = sc.RouteID
    JOIN Stop st ON st.StopID = sc.StopID
    JOIN Sites s ON st.SIName = s.SIName AND s.PhoneNumber = st.PhoneNumber 
    WHERE s.category = 'Library'
      AND sc.Date IN ('2023-09-05', '2023-09-06')
    GROUP BY sc.Date, p.Occupation)
SELECT Occupation, Date
FROM (SELECT Date,Occupation,occurrences,
        RANK() OVER (PARTITION BY Date ORDER BY occurrences DESC) AS rank
    FROM OccupationVisits
    ) RankedVisits
WHERE rank = 1;
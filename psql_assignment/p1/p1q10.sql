-- Ethan Tang, tangeth4, 1009681471
SELECT sc.RouteID, st.SName AS StopName, sc.ArrivalTime
FROM Schedule sc
JOIN Stop st ON sc.StopID = st.StopID
JOIN Sites s ON st.SIName = s.SIName AND st.PhoneNumber = s.PhoneNumber
JOIN Event e ON e.SIName = s.SIName AND e.PhoneNumber = s.PhoneNumber
WHERE e.EName = 'Jedi Knight Colloquium'
  AND e.Date = '2023-09-06'
  AND sc.ArrivalTime BETWEEN '16:00:00' AND '18:00:00'
  AND e.Time = '18:30:00';

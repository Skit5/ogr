-- MySQL dump 10.13  Distrib 5.7.17, for Win64 (x86_64)
--
-- Host: localhost    Database: dbogr
-- ------------------------------------------------------
-- Server version	5.7.19-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `constructors`
--

DROP TABLE IF EXISTS `constructors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `constructors` (
  `idConstructor` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`idConstructor`)
) ENGINE=InnoDB AUTO_INCREMENT=17 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `constructors`
--

LOCK TABLES `constructors` WRITE;
/*!40000 ALTER TABLE `constructors` DISABLE KEYS */;
INSERT INTO `constructors` VALUES (1,'Autre'),(2,'Aprilia'),(3,'BMW'),(4,'Ducati'),(5,'Harley-Davidson'),(6,'Honda'),(7,'Husqvarna'),(8,'Indian'),(9,'Kawasaki'),(10,'KTM'),(11,'Moto-Guzzi'),(12,'MV-Agusta'),(13,'Suzuki'),(14,'Triumph'),(15,'Victory'),(16,'Yamaha');
/*!40000 ALTER TABLE `constructors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `releves`
--

DROP TABLE IF EXISTS `releves`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `releves` (
  `vehicule` int(11) NOT NULL,
  `w` int(11) NOT NULL,
  `C` double DEFAULT NULL,
  PRIMARY KEY (`w`,`vehicule`),
  KEY `fk_Releves_Vehicules_idx` (`vehicule`),
  KEY `pk_Releves` (`w`,`vehicule`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `releves`
--

LOCK TABLES `releves` WRITE;
/*!40000 ALTER TABLE `releves` DISABLE KEYS */;
INSERT INTO `releves` VALUES (1,2000,7.84299408),(1,2200,7.95018728),(1,2400,8.030445759),(1,2600,8.093935651),(1,2800,8.148906728),(1,3000,8.201833795),(1,3200,8.257558077),(1,3400,8.319428613),(1,3600,8.389443647),(1,3800,8.468392021),(1,4000,8.55599456),(1,4200,8.651045472),(1,4400,8.751553731),(1,4600,8.854884476),(1,4800,8.957900396),(1,5000,9.057103125),(1,5200,9.148774631),(1,5400,9.229118609),(1,5600,9.294401872),(1,5800,9.341095742),(1,6000,9.36601744),(1,6200,9.366471481),(1,6400,9.34039106),(1,6600,9.286479448),(1,6800,9.204351383),(1,7000,9.094674455),(1,7200,8.959310507),(1,7400,8.801457018),(1,7600,8.6257885),(1,7800,8.438597885),(1,8000,8.24793792),(1,8200,8.063762555),(1,8400,7.898068337);
/*!40000 ALTER TABLE `releves` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `vehicules`
--

DROP TABLE IF EXISTS `vehicules`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicules` (
  `idVehicule` int(11) NOT NULL AUTO_INCREMENT,
  `nom` varchar(255) NOT NULL,
  `details` mediumtext,
  `constructor` int(11) NOT NULL,
  `constructionDate` date DEFAULT NULL,
  `insertDate` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`idVehicule`),
  KEY `fk_Vehicules_Constructors1_idx` (`constructor`),
  CONSTRAINT `fk_Vehicules_Constructors1` FOREIGN KEY (`constructor`) REFERENCES `constructors` (`idConstructor`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `vehicules`
--

LOCK TABLES `vehicules` WRITE;
/*!40000 ALTER TABLE `vehicules` DISABLE KEYS */;
INSERT INTO `vehicules` VALUES (1,'KBX-200','Moto Sample',3,'2006-01-01','2017-09-07 03:00:01');
/*!40000 ALTER TABLE `vehicules` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-09-07  3:01:34

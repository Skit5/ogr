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
-- Table structure for table `releves`
--

DROP TABLE IF EXISTS `releves`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `releves` (
  `idReleve` bigint(20) NOT NULL AUTO_INCREMENT,
  `vehicule` varchar(255) NOT NULL,
  `w` int(11) DEFAULT NULL,
  `P` double DEFAULT NULL,
  PRIMARY KEY (`idReleve`),
  KEY `fk_Releves_Vehicules_idx` (`vehicule`),
  CONSTRAINT `fk_Releves_Vehicules` FOREIGN KEY (`vehicule`) REFERENCES `vehicules` (`nom`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=199 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `releves`
--

LOCK TABLES `releves` WRITE;
/*!40000 ALTER TABLE `releves` DISABLE KEYS */;
INSERT INTO `releves` VALUES (67,'Motor Sample 001',0,0),(68,'Motor Sample 001',200,0),(69,'Motor Sample 001',400,0),(70,'Motor Sample 001',600,0),(71,'Motor Sample 001',800,0),(72,'Motor Sample 001',1000,0),(73,'Motor Sample 001',1200,0),(74,'Motor Sample 001',1400,0),(75,'Motor Sample 001',1600,0),(76,'Motor Sample 001',1800,0),(77,'Motor Sample 001',2000,32.0499799),(78,'Motor Sample 001',2200,34.10855407),(79,'Motor Sample 001',2400,35.83867788),(80,'Motor Sample 001',2600,37.31484845),(81,'Motor Sample 001',2800,38.60435875),(82,'Motor Sample 001',3000,39.76756725),(83,'Motor Sample 001',3200,40.85816745),(84,'Motor Sample 001',3400,41.92345753),(85,'Motor Sample 001',3600,43.00460991),(86,'Motor Sample 001',3800,44.13694087),(87,'Motor Sample 001',4000,45.35018013),(88,'Motor Sample 001',4200,46.66874044),(89,'Motor Sample 001',4400,48.11198721),(90,'Motor Sample 001',4600,49.69450806),(91,'Motor Sample 001',4800,51.42638244),(92,'Motor Sample 001',5000,53.31345125),(93,'Motor Sample 001',5200,55.35758637),(94,'Motor Sample 001',5400,57.55696033),(95,'Motor Sample 001',5600,59.90631584),(96,'Motor Sample 001',5800,62.39723545),(97,'Motor Sample 001',6000,65.01841107),(98,'Motor Sample 001',6200,67.75591365),(99,'Motor Sample 001',6400,70.59346271),(100,'Motor Sample 001',6600,73.51269597),(101,'Motor Sample 001',6800,76.49343892),(102,'Motor Sample 001',7000,79.51397445),(103,'Motor Sample 001',7200,82.55131242),(104,'Motor Sample 001',7400,85.58145924),(105,'Motor Sample 001',7600,88.57968753),(106,'Motor Sample 001',7800,91.52080564),(107,'Motor Sample 001',8000,94.3794273),(108,'Motor Sample 001',8200,97.13024117),(109,'Motor Sample 001',8400,99.74828049),(110,'Motor Sample 001',8600,102.2091926),(111,'Motor Sample 001',8800,104.4895087),(112,'Motor Sample 001',9000,106.5669132),(113,'Motor Sample 001',9200,108.4205134),(114,'Motor Sample 001',9400,110.0311094),(115,'Motor Sample 001',9600,111.381463),(116,'Motor Sample 001',9800,112.4565682),(117,'Motor Sample 001',10000,113.24392),(118,'Motor Sample 001',10200,113.7337843),(119,'Motor Sample 001',10400,113.9194678),(120,'Motor Sample 001',10600,113.7975869),(121,'Motor Sample 001',10800,113.3683379),(122,'Motor Sample 001',11000,112.6357666),(123,'Motor Sample 001',11200,111.6080374),(124,'Motor Sample 001',11400,110.2977032),(125,'Motor Sample 001',11600,108.7219751),(126,'Motor Sample 001',11800,106.9029919),(127,'Motor Sample 001',12000,104.8680895),(128,'Motor Sample 001',12200,102.6500709),(129,'Motor Sample 001',12400,0),(130,'Motor Sample 001',12600,0),(131,'Motor Sample 001',12800,0),(132,'Motor Sample 001',13000,0),(133,'Motor Sample 002',0,0),(134,'Motor Sample 002',200,0),(135,'Motor Sample 002',400,0),(136,'Motor Sample 002',600,0),(137,'Motor Sample 002',800,0),(138,'Motor Sample 002',1000,0),(139,'Motor Sample 002',1200,0),(140,'Motor Sample 002',1400,0),(141,'Motor Sample 002',1600,0),(142,'Motor Sample 002',1800,0),(143,'Motor Sample 002',2000,31.16785248),(144,'Motor Sample 002',2200,33.54836469),(145,'Motor Sample 002',2400,35.89284335),(146,'Motor Sample 002',2600,38.24241567),(147,'Motor Sample 002',2800,40.62921843),(148,'Motor Sample 002',3000,43.07702557),(149,'Motor Sample 002',3200,45.60187594),(150,'Motor Sample 002',3400,48.21270087),(151,'Motor Sample 002',3600,50.91195188),(152,'Motor Sample 002',3800,53.6962283),(153,'Motor Sample 002',4000,56.55690496),(154,'Motor Sample 002',4200,59.48075981),(155,'Motor Sample 002',4400,62.45060161),(156,'Motor Sample 002',4600,65.44589754),(157,'Motor Sample 002',4800,68.4434009),(158,'Motor Sample 002',5000,71.41777875),(159,'Motor Sample 002',5200,74.34223956),(160,'Motor Sample 002',5400,77.18916085),(161,'Motor Sample 002',5600,79.9307169),(162,'Motor Sample 002',5800,82.53950634),(163,'Motor Sample 002',6000,84.98917984),(164,'Motor Sample 002',6200,87.25506776),(165,'Motor Sample 002',6400,89.31480781),(166,'Motor Sample 002',6600,91.1489727),(167,'Motor Sample 002',6800,92.74169778),(168,'Motor Sample 002',7000,94.08130873),(169,'Motor Sample 002',7200,95.16094919),(170,'Motor Sample 002',7400,95.97920841),(171,'Motor Sample 002',7600,96.54074893),(172,'Motor Sample 002',7800,96.85693421),(173,'Motor Sample 002',8000,96.94645632),(174,'Motor Sample 002',8200,96.83596354),(175,'Motor Sample 002',8400,96.56068808),(176,'Motor Sample 002',8600,0),(177,'Motor Sample 002',8800,0),(178,'Motor Sample 002',9000,0),(179,'Motor Sample 002',9200,0),(180,'Motor Sample 002',9400,0),(181,'Motor Sample 002',9600,0),(182,'Motor Sample 002',9800,0),(183,'Motor Sample 002',10000,0),(184,'Motor Sample 002',10200,0),(185,'Motor Sample 002',10400,0),(186,'Motor Sample 002',10600,0),(187,'Motor Sample 002',10800,0),(188,'Motor Sample 002',11000,0),(189,'Motor Sample 002',11200,0),(190,'Motor Sample 002',11400,0),(191,'Motor Sample 002',11600,0),(192,'Motor Sample 002',11800,0),(193,'Motor Sample 002',12000,0),(194,'Motor Sample 002',12200,0),(195,'Motor Sample 002',12400,0),(196,'Motor Sample 002',12600,0),(197,'Motor Sample 002',12800,0),(198,'Motor Sample 002',13000,0);
/*!40000 ALTER TABLE `releves` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `vehicules`
--

DROP TABLE IF EXISTS `vehicules`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vehicules` (
  `nom` varchar(255) NOT NULL,
  `details` mediumtext,
  `insertDate` datetime DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`nom`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `vehicules`
--

LOCK TABLES `vehicules` WRITE;
/*!40000 ALTER TABLE `vehicules` DISABLE KEYS */;
INSERT INTO `vehicules` VALUES ('Motor Sample 001','Exemple de véhicule','2017-08-31 20:13:32'),('Motor Sample 002','Autre Exemple de véhicule','2017-09-02 07:40:23');
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

-- Dump completed on 2017-09-02  7:41:18
